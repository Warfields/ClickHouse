#pragma once

#include <Common/config.h>

#if USE_HIVE
#include <IO/PeekableReadBuffer.h>
#include <Processors/Formats/Impl/CSVRowInputFormat.h>


namespace DB
{

/// A stream for input data in Hive Text format.
/// Parallel parsing is disabled currently.
class HiveTextRowInputFormat : public CSVRowInputFormat
{
public:
    HiveTextRowInputFormat(
        const Block & header_,
        ReadBuffer & in_,
        const Params & params_,
        const FormatSettings & format_settings_);

    String getName() const override { return "HiveTextRowInputFormat"; }

protected:
    std::vector<String> readNames() override;
    std::vector<String> readTypes() override;

private:
    PeekableReadBuffer buf;
    std::vector<String> input_field_names;
};
}

#endif
