#include <Common/MemoryWorker.h>

#include <IO/ReadBufferFromFile.h>
#include <IO/ReadBufferFromFileDescriptor.h>
#include <IO/ReadHelpers.h>
#include <base/cgroupsv2.h>
#include <Common/Jemalloc.h>
#include <Common/MemoryTracker.h>
#include <Common/ProfileEvents.h>
#include <Common/formatReadable.h>
#include <Common/logger_useful.h>

#include <filesystem>
#include <memory>
#include <optional>

namespace fs = std::filesystem;

namespace ProfileEvents
{
    extern const Event MemoryAllocatorPurge;
    extern const Event MemoryAllocatorPurgeTimeMicroseconds;
    extern const Event MemoryWorkerRun;
    extern const Event MemoryWorkerRunElapsedMicroseconds;
}

namespace DB
{

namespace ErrorCodes
{
    extern const int FILE_DOESNT_EXIST;
    extern const int LOGICAL_ERROR;
}

struct ICgroupsReader
{
    virtual ~ICgroupsReader() = default;

    virtual uint64_t readMemoryUsage() = 0;
};

namespace
{

#if defined(OS_LINUX)
/// Format is
///   kernel 5
///   rss 15
///   [...]
uint64_t readMetricFromStatFile(ReadBufferFromFile & buf, const std::string & key)
{
    while (!buf.eof())
    {
        std::string current_key;
        readStringUntilWhitespace(current_key, buf);
        if (current_key != key)
        {
            std::string dummy;
            readStringUntilNewlineInto(dummy, buf);
            buf.ignore();
            continue;
        }

        assertChar(' ', buf);
        uint64_t value = 0;
        readIntText(value, buf);
        return value;
    }

    LOG_ERROR(getLogger("CgroupsReader"), "Cannot find '{}' in '{}'", key, buf.getFileName());
    return 0;
}

struct CgroupsV1Reader : ICgroupsReader
{
    explicit CgroupsV1Reader(const fs::path & stat_file_dir) : buf(stat_file_dir / "memory.stat") { }

    uint64_t readMemoryUsage() override
    {
        std::lock_guard lock(mutex);
        buf.rewind();
        return readMetricFromStatFile(buf, "rss");
    }

private:
    std::mutex mutex;
    ReadBufferFromFile buf TSA_GUARDED_BY(mutex);
};

struct CgroupsV2Reader : ICgroupsReader
{
    explicit CgroupsV2Reader(const fs::path & stat_file_dir) : stat_buf(stat_file_dir / "memory.stat") { }

    uint64_t readMemoryUsage() override
    {
        std::lock_guard lock(mutex);
        stat_buf.rewind();
        return readMetricFromStatFile(stat_buf, "anon");
    }

private:
    std::mutex mutex;
    ReadBufferFromFile stat_buf TSA_GUARDED_BY(mutex);
};

/// Caveats:
/// - All of the logic in this file assumes that the current process is the only process in the
///   containing cgroup (or more precisely: the only process with significant memory consumption).
///   If this is not the case, then other processe's memory consumption may affect the internal
///   memory tracker ...
/// - Cgroups v1 and v2 allow nested cgroup hierarchies. As v1 is deprecated for over half a
///   decade and will go away at some point, hierarchical detection is only implemented for v2.
/// - I did not test what happens if a host has v1 and v2 simultaneously enabled. I believe such
///   systems existed only for a short transition period.

std::optional<std::string> getCgroupsV2Path()
{
    if (!cgroupsV2Enabled())
        return {};

    if (!cgroupsV2MemoryControllerEnabled())
        return {};

    fs::path current_cgroup = cgroupV2PathOfProcess();
    if (current_cgroup.empty())
        return {};

    /// Return the bottom-most nested current memory file. If there is no such file at the current
    /// level, try again at the parent level as memory settings are inherited.
    while (current_cgroup != default_cgroups_mount.parent_path())
    {
        const auto current_path = current_cgroup / "memory.current";
        const auto stat_path = current_cgroup / "memory.stat";
        if (fs::exists(current_path) && fs::exists(stat_path))
            return {current_cgroup};
        current_cgroup = current_cgroup.parent_path();
    }
    return {};
}

std::optional<std::string> getCgroupsV1Path()
{
    auto path = default_cgroups_mount / "memory/memory.stat";
    if (!fs::exists(path))
        return {};
    return {default_cgroups_mount / "memory"};
}

enum class CgroupsVersion : uint8_t
{
    V1,
    V2
};

std::pair<std::string, CgroupsVersion> getCgroupsPath()
{
    auto v2_path = getCgroupsV2Path();
    if (v2_path.has_value())
        return {*v2_path, CgroupsVersion::V2};

    auto v1_path = getCgroupsV1Path();
    if (v1_path.has_value())
        return {*v1_path, CgroupsVersion::V1};

    throw Exception(ErrorCodes::FILE_DOESNT_EXIST, "Cannot find cgroups v1 or v2 current memory file");
}

std::shared_ptr<ICgroupsReader> createCgroupsReader()
{
    const auto [cgroup_path, version] = getCgroupsPath();
    LOG_INFO(
        getLogger("CgroupsReader"),
        "Will create cgroup reader from '{}' (cgroups version: {})",
        cgroup_path,
        (version == CgroupsVersion::V1) ? "v1" : "v2");

    if (version == CgroupsVersion::V2)
        return std::make_shared<CgroupsV2Reader>(cgroup_path);
    else
    {
        chassert(version == CgroupsVersion::V1);
        return std::make_shared<CgroupsV1Reader>(cgroup_path);
    }

}
#endif

std::string_view sourceToString(MemoryWorker::MemoryUsageSource source)
{
    switch (source)
    {
        case MemoryWorker::MemoryUsageSource::Cgroups: return "Cgroups";
        case MemoryWorker::MemoryUsageSource::Jemalloc: return "Jemalloc";
        case MemoryWorker::MemoryUsageSource::None: return "None";
    }
}

}

/// We try to pick the best possible supported source for reading memory usage.
/// Supported sources in order of priority
/// - reading from cgroups' pseudo-files (fastest and most accurate)
/// - reading jemalloc's resident stat (doesn't take into account allocations that didn't use jemalloc)
/// Also, different tick rates are used because not all options are equally fast
MemoryWorker::MemoryWorker(uint64_t period_ms_)
    : log(getLogger("MemoryWorker"))
    , period_ms(period_ms_)
{
#if defined(OS_LINUX)
    try
    {
        static constexpr uint64_t cgroups_memory_usage_tick_ms{50};

        cgroups_reader = createCgroupsReader();
        source = MemoryUsageSource::Cgroups;
        if (period_ms == 0)
            period_ms = cgroups_memory_usage_tick_ms;

        return;
    }
    catch (...)
    {
        tryLogCurrentException(log, "Cannot use cgroups reader");
    }
#endif

#if USE_JEMALLOC
    static constexpr uint64_t jemalloc_memory_usage_tick_ms{100};

    source = MemoryUsageSource::Jemalloc;
    if (period_ms == 0)
        period_ms = jemalloc_memory_usage_tick_ms;
#endif
}

MemoryWorker::MemoryUsageSource MemoryWorker::getSource()
{
    return source;
}

void MemoryWorker::start()
{
    if (source == MemoryUsageSource::None)
        return;

    LOG_INFO(
        getLogger("MemoryWorker"),
        "Starting background memory thread with period of {}ms, using {} as source",
        period_ms,
        sourceToString(source));
    background_thread = ThreadFromGlobalPool([this] { backgroundThread(); });
}

MemoryWorker::~MemoryWorker()
{
    {
        std::unique_lock lock(mutex);
        shutdown = true;
    }
    cv.notify_all();

    if (background_thread.joinable())
        background_thread.join();
}

uint64_t MemoryWorker::getMemoryUsage()
{
    switch (source)
    {
        case MemoryUsageSource::Cgroups:
            return cgroups_reader != nullptr ? cgroups_reader->readMemoryUsage() : 0;
        case MemoryUsageSource::Jemalloc:
#if USE_JEMALLOC
            return resident_mib.getValue();
#else
            return 0;
#endif
        case MemoryUsageSource::None:
            throw DB::Exception(ErrorCodes::LOGICAL_ERROR, "Trying to fetch memory usage while no memory source can be used");
    }
}

void MemoryWorker::backgroundThread()
{
    std::chrono::milliseconds chrono_period_ms{period_ms};
    [[maybe_unused]] bool first_run = true;
    std::unique_lock lock(mutex);
    while (true)
    {
        cv.wait_for(lock, chrono_period_ms, [this] { return shutdown; });
        if (shutdown)
            return;

        Stopwatch total_watch;

#if USE_JEMALLOC
        if (source == MemoryUsageSource::Jemalloc)
            epoch_mib.setValue(0);
#endif

        Int64 resident = getMemoryUsage();
        MemoryTracker::updateRSS(resident);

#if USE_JEMALLOC
        if (resident > total_memory_tracker.getHardLimit())
        {
            Stopwatch purge_watch;
            purge_mib.run();
            ProfileEvents::increment(ProfileEvents::MemoryAllocatorPurge);
            ProfileEvents::increment(ProfileEvents::MemoryAllocatorPurgeTimeMicroseconds, purge_watch.elapsedMicroseconds());
        }

        if (unlikely(first_run || total_memory_tracker.get() < 0))
        {
            if (source != MemoryUsageSource::Jemalloc)
                epoch_mib.setValue(0);

            MemoryTracker::updateAllocated(allocated_mib.getValue());
        }
#endif

        ProfileEvents::increment(ProfileEvents::MemoryWorkerRun);
        ProfileEvents::increment(ProfileEvents::MemoryWorkerRunElapsedMicroseconds, total_watch.elapsedMicroseconds());
        first_run = false;
    }
}

}
