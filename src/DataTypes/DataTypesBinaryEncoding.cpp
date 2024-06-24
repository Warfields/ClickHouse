#include <DataTypes/DataTypesBinaryEncoding.h>
#include <DataTypes/DataTypeDateTime64.h>
#include <DataTypes/DataTypeFixedString.h>
#include <DataTypes/DataTypeEnum.h>
#include <DataTypes/DataTypesDecimal.h>
#include <DataTypes/DataTypeArray.h>
#include <DataTypes/DataTypeTuple.h>
#include <DataTypes/DataTypeNullable.h>
#include <DataTypes/DataTypeFunction.h>
#include <DataTypes/DataTypeLowCardinality.h>
#include <DataTypes/DataTypeMap.h>
#include <DataTypes/DataTypeObject.h>
#include <DataTypes/DataTypeVariant.h>
#include <DataTypes/DataTypeString.h>
#include <DataTypes/DataTypeUUID.h>
#include <DataTypes/DataTypeSet.h>
#include <DataTypes/DataTypeInterval.h>
#include <DataTypes/DataTypeIPv4andIPv6.h>
#include <DataTypes/DataTypeAggregateFunction.h>
#include <DataTypes/DataTypeCustomSimpleAggregateFunction.h>
#include <DataTypes/DataTypeNothing.h>
#include <DataTypes/DataTypeDynamic.h>
#include <DataTypes/DataTypeNested.h>
#include <DataTypes/DataTypeFactory.h>
#include <AggregateFunctions/IAggregateFunction.h>
#include <AggregateFunctions/AggregateFunctionFactory.h>
#include <Parsers/NullsAction.h>
#include <IO/WriteBuffer.h>
#include <IO/ReadBuffer.h>
#include <IO/ReadBufferFromString.h>
#include <IO/WriteHelpers.h>
#include <IO/ReadHelpers.h>
#include <Common/FieldBinaryEncoding.h>
#include <Common/assert_cast.h>

namespace DB
{

namespace ErrorCodes
{
    extern const int UNSUPPORTED_METHOD;
    extern const int INCORRECT_DATA;
}

namespace
{

enum class BinaryTypeIndex : uint8_t
{
    Nothing = 0x00,
    UInt8 = 0x01,
    UInt16 = 0x02,
    UInt32 = 0x03,
    UInt64 = 0x04,
    UInt128 = 0x05,
    UInt256 = 0x06,
    Int8 = 0x07,
    Int16 = 0x08,
    Int32 = 0x09,
    Int64 = 0x0A,
    Int128 = 0x0B,
    Int256 = 0x0C,
    Float32 = 0x0D,
    Float64 = 0x0E,
    Date = 0x0F,
    Date32 = 0x10,
    DateTimeUTC = 0x11,
    DateTimeWithTimezone = 0x12,
    DateTime64UTC = 0x13,
    DateTime64WithTimezone = 0x14,
    String = 0x15,
    FixedString = 0x16,
    Enum8 = 0x17,
    Enum16 = 0x18,
    Decimal32 = 0x19,
    Decimal64 = 0x1A,
    Decimal128 = 0x1B,
    Decimal256 = 0x1C,
    UUID = 0x1D,
    Array = 0x1E,
    UnnamedTuple = 0x1F,
    NamedTuple = 0x20,
    Set = 0x21,
    Interval = 0x22,
    Nullable = 0x23,
    Function = 0x24,
    AggregateFunction = 0x25,
    LowCardinality = 0x26,
    Map = 0x27,
    Object = 0x28,
    IPv4 = 0x29,
    IPv6 = 0x2A,
    Variant = 0x2B,
    Dynamic = 0x2C,
    Custom = 0x2D,
    Bool = 0x2E,
    SimpleAggregateFunction = 0x2F,
    Nested = 0x30,
};

BinaryTypeIndex getBinaryTypeIndex(const DataTypePtr & type)
{
    /// By default custom types don't have their own BinaryTypeIndex.
    if (type->hasCustomName())
    {
        /// Some widely used custom types have separate BinaryTypeIndex for better serialization.
        /// Right now it's Bool, SimpleAggregateFunction and Nested types.
        /// TODO: Consider adding BinaryTypeIndex for more custom types.

        if (isBool(type))
            return BinaryTypeIndex::Bool;

        if (typeid_cast<const DataTypeCustomSimpleAggregateFunction *>(type->getCustomName()))
            return BinaryTypeIndex::SimpleAggregateFunction;

        if (isNested(type))
            return BinaryTypeIndex::Nested;

        return BinaryTypeIndex::Custom;
    }

    switch (type->getTypeId())
    {
        case TypeIndex::Nothing:
            return BinaryTypeIndex::Nothing;
        case TypeIndex::UInt8:
            return BinaryTypeIndex::UInt8;
        case TypeIndex::UInt16:
            return BinaryTypeIndex::UInt16;
        case TypeIndex::UInt32:
            return BinaryTypeIndex::UInt32;
        case TypeIndex::UInt64:
            return BinaryTypeIndex::UInt64;
        case TypeIndex::UInt128:
            return BinaryTypeIndex::UInt128;
        case TypeIndex::UInt256:
            return BinaryTypeIndex::UInt256;
        case TypeIndex::Int8:
            return BinaryTypeIndex::Int8;
        case TypeIndex::Int16:
            return BinaryTypeIndex::Int16;
        case TypeIndex::Int32:
            return BinaryTypeIndex::Int32;
        case TypeIndex::Int64:
            return BinaryTypeIndex::Int64;
        case TypeIndex::Int128:
            return BinaryTypeIndex::Int128;
        case TypeIndex::Int256:
            return BinaryTypeIndex::Int256;
        case TypeIndex::Float32:
            return BinaryTypeIndex::Float32;
        case TypeIndex::Float64:
            return BinaryTypeIndex::Float64;
        case TypeIndex::Date:
            return BinaryTypeIndex::Date;
        case TypeIndex::Date32:
            return BinaryTypeIndex::Date32;
        case TypeIndex::DateTime:
            if (assert_cast<const DataTypeDateTime &>(*type).hasExplicitTimeZone())
                return BinaryTypeIndex::DateTimeWithTimezone;
            return BinaryTypeIndex::DateTimeUTC;
        case TypeIndex::DateTime64:
            if (assert_cast<const DataTypeDateTime64 &>(*type).hasExplicitTimeZone())
                return BinaryTypeIndex::DateTime64WithTimezone;
            return BinaryTypeIndex::DateTime64UTC;
        case TypeIndex::String:
            return BinaryTypeIndex::String;
        case TypeIndex::FixedString:
            return BinaryTypeIndex::FixedString;
        case TypeIndex::Enum8:
            return BinaryTypeIndex::Enum8;
        case TypeIndex::Enum16:
            return BinaryTypeIndex::Enum16;
        case TypeIndex::Decimal32:
            return BinaryTypeIndex::Decimal32;
        case TypeIndex::Decimal64:
            return BinaryTypeIndex::Decimal64;
        case TypeIndex::Decimal128:
            return BinaryTypeIndex::Decimal128;
        case TypeIndex::Decimal256:
            return BinaryTypeIndex::Decimal256;
        case TypeIndex::UUID:
            return BinaryTypeIndex::UUID;
        case TypeIndex::Array:
            return BinaryTypeIndex::Array;
        case TypeIndex::Tuple:
        {
            const auto & tuple_type = assert_cast<const DataTypeTuple &>(*type);
            if (tuple_type.haveExplicitNames())
                return BinaryTypeIndex::NamedTuple;
            return BinaryTypeIndex::UnnamedTuple;
        }
        case TypeIndex::Set:
            return BinaryTypeIndex::Set;
        case TypeIndex::Interval:
            return BinaryTypeIndex::Interval;
        case TypeIndex::Nullable:
            return BinaryTypeIndex::Nullable;
        case TypeIndex::Function:
            return BinaryTypeIndex::Function;
        case TypeIndex::AggregateFunction:
            return BinaryTypeIndex::AggregateFunction;
        case TypeIndex::LowCardinality:
            return BinaryTypeIndex::LowCardinality;
        case TypeIndex::Map:
            return BinaryTypeIndex::Map;
        case TypeIndex::Object:
            return BinaryTypeIndex::Object;
        case TypeIndex::IPv4:
            return BinaryTypeIndex::IPv4;
        case TypeIndex::IPv6:
            return BinaryTypeIndex::IPv6;
        case TypeIndex::Variant:
            return BinaryTypeIndex::Variant;
        case TypeIndex::Dynamic:
            return BinaryTypeIndex::Dynamic;
        /// JSONPaths is used only during schema inference and cannot be used anywhere else.
        case TypeIndex::JSONPaths:
            throw Exception(ErrorCodes::UNSUPPORTED_METHOD, "Binary encoding of type JSONPaths is not supported");
    }
}

template <typename T>
void encodeEnumValues(const DataTypePtr & type, WriteBuffer & buf)
{
    const auto & enum_type = assert_cast<const DataTypeEnum<T> &>(*type);
    const auto & values = enum_type.getValues();
    writeVarUInt(values.size(), buf);
    for (const auto & [name, value] : values)
    {
        writeStringBinary(name, buf);
        writeBinaryLittleEndian(value, buf);
    }
}

template <typename T>
DataTypePtr decodeEnum(ReadBuffer & buf)
{
    typename DataTypeEnum<T>::Values values;
    size_t size;
    readVarUInt(size, buf);
    for (size_t i = 0; i != size; ++i)
    {
        String name;
        readStringBinary(name, buf);
        T value;
        readBinaryLittleEndian(value, buf);
        values.emplace_back(name, value);
    }

    return std::make_shared<DataTypeEnum<T>>(values);
}

template <typename T>
void encodeDecimal(const DataTypePtr & type, WriteBuffer & buf)
{
    const auto & decimal_type = assert_cast<const DataTypeDecimal<T> &>(*type);
    /// Both precision and scale should be less than 76, so we can decode it in 1 byte.
    writeBinary(UInt8(decimal_type.getPrecision()), buf);
    writeBinary(UInt8(decimal_type.getScale()), buf);
}

template <typename T>
DataTypePtr decodeDecimal(ReadBuffer & buf)
{
    UInt8 precision;
    readBinary(precision, buf);
    UInt8 scale;
    readBinary(scale, buf);
    return std::make_shared<DataTypeDecimal<T>>(precision, scale);
}

void encodeAggregateFunction(const String & function_name, const Array & parameters, const DataTypes & arguments_types, WriteBuffer & buf)
{
    writeStringBinary(function_name, buf);
    writeVarUInt(parameters.size(), buf);
    for (const auto & param : parameters)
        encodeField(param, buf);
    writeVarUInt(arguments_types.size(), buf);
    for (const auto & argument_type : arguments_types)
        encodeDataType(argument_type, buf);
}

std::tuple<AggregateFunctionPtr, Array, DataTypes> decodeAggregateFunction(ReadBuffer & buf)
{
    String function_name;
    readStringBinary(function_name, buf);
    size_t num_parameters;
    readVarUInt(num_parameters, buf);
    Array parameters;
    parameters.reserve(num_parameters);
    for (size_t i = 0; i != num_parameters; ++i)
        parameters.push_back(decodeField(buf));
    size_t num_arguments;
    readVarUInt(num_arguments, buf);
    DataTypes arguments_types;
    arguments_types.reserve(num_arguments);
    for (size_t i = 0; i != num_arguments; ++i)
        arguments_types.push_back(decodeDataType(buf));
    AggregateFunctionProperties properties;
    auto action = NullsAction::EMPTY;
    auto function = AggregateFunctionFactory::instance().get(function_name, action, arguments_types, parameters, properties);
    return {function, parameters, arguments_types};
}

}

void encodeDataType(const DataTypePtr & type, WriteBuffer & buf)
{
    /// First, write the BinaryTypeIndex byte.
    auto binary_type_index = getBinaryTypeIndex(type);
    buf.write(UInt8(binary_type_index));
    /// Then, write additional information depending on the data type.
    switch (binary_type_index)
    {
        case BinaryTypeIndex::DateTimeWithTimezone:
        {
            const auto & datetime_type = assert_cast<const DataTypeDateTime &>(*type);
            writeStringBinary(datetime_type.getTimeZone().getTimeZone(), buf);
            break;
        }
        case BinaryTypeIndex::DateTime64UTC:
        {
            const auto & datetime64_type = assert_cast<const DataTypeDateTime64 &>(*type);
            /// Maximum scale for DateTime64 is 9, so we can write it as 1 byte.
            buf.write(UInt8(datetime64_type.getScale()));
            break;
        }
        case BinaryTypeIndex::DateTime64WithTimezone:
        {
            const auto & datetime64_type = assert_cast<const DataTypeDateTime64 &>(*type);
            buf.write(UInt8(datetime64_type.getScale()));
            writeStringBinary(datetime64_type.getTimeZone().getTimeZone(), buf);
            break;
        }
        case BinaryTypeIndex::FixedString:
        {
            const auto & fixed_string_type = assert_cast<const DataTypeFixedString &>(*type);
            writeVarUInt(fixed_string_type.getN(), buf);
            break;
        }
        case BinaryTypeIndex::Enum8:
        {
            encodeEnumValues<Int8>(type, buf);
            break;
        }
        case BinaryTypeIndex::Enum16:
        {
            encodeEnumValues<Int16>(type, buf);
            break;
        }
        case BinaryTypeIndex::Decimal32:
        {
            encodeDecimal<Decimal32>(type, buf);
            break;
        }
        case BinaryTypeIndex::Decimal64:
        {
            encodeDecimal<Decimal64>(type, buf);
            break;
        }
        case BinaryTypeIndex::Decimal128:
        {
            encodeDecimal<Decimal128>(type, buf);
            break;
        }
        case BinaryTypeIndex::Decimal256:
        {
            encodeDecimal<Decimal256>(type, buf);
            break;
        }
        case BinaryTypeIndex::Array:
        {
            const auto & array_type = assert_cast<const DataTypeArray &>(*type);
            encodeDataType(array_type.getNestedType(), buf);
            break;
        }
        case BinaryTypeIndex::NamedTuple:
        {
            const auto & tuple_type = assert_cast<const DataTypeTuple &>(*type);
            const auto & types = tuple_type.getElements();
            const auto & names = tuple_type.getElementNames();
            writeVarUInt(types.size(), buf);
            for (size_t i = 0; i != types.size(); ++i)
            {
                writeStringBinary(names[i], buf);
                encodeDataType(types[i], buf);
            }
            break;
        }
        case BinaryTypeIndex::UnnamedTuple:
        {
            const auto & tuple_type = assert_cast<const DataTypeTuple &>(*type);
            const auto & element_types = tuple_type.getElements();
            writeVarUInt(element_types.size(), buf);
            for (const auto & element_type : element_types)
                encodeDataType(element_type, buf);
            break;
        }
        case BinaryTypeIndex::Interval:
        {
            const auto & interval_type = assert_cast<const DataTypeInterval &>(*type);
            writeBinary(UInt8(interval_type.getKind().kind), buf);
            break;
        }
        case BinaryTypeIndex::Nullable:
        {
            const auto & nullable_type = assert_cast<const DataTypeNullable &>(*type);
            encodeDataType(nullable_type.getNestedType(), buf);
            break;
        }
        case BinaryTypeIndex::Function:
        {
            const auto & function_type = assert_cast<const DataTypeFunction &>(*type);
            const auto & arguments_types = function_type.getArgumentTypes();
            const auto & return_type = function_type.getReturnType();
            writeVarUInt(arguments_types.size(), buf);
            for (const auto & argument_type : arguments_types)
                encodeDataType(argument_type, buf);
            encodeDataType(return_type, buf);
            break;
        }
        case BinaryTypeIndex::LowCardinality:
        {
            const auto & low_cardinality_type = assert_cast<const DataTypeLowCardinality &>(*type);
            encodeDataType(low_cardinality_type.getDictionaryType(), buf);
            break;
        }
        case BinaryTypeIndex::Map:
        {
            const auto & map_type = assert_cast<const DataTypeMap &>(*type);
            encodeDataType(map_type.getKeyType(), buf);
            encodeDataType(map_type.getValueType(), buf);
            break;
        }
        case BinaryTypeIndex::Object:
        {
            const auto & object_deprecated_type = assert_cast<const DataTypeObject &>(*type);
            writeBinary(object_deprecated_type.hasNullableSubcolumns(), buf);
            writeStringBinary(object_deprecated_type.getSchemaFormat(), buf);
            break;
        }
        case BinaryTypeIndex::Variant:
        {
            const auto & variant_type = assert_cast<const DataTypeVariant &>(*type);
            const auto & variants = variant_type.getVariants();
            writeVarUInt(variants.size(), buf);
            for (const auto & variant : variants)
                encodeDataType(variant, buf);
            break;
        }
        case BinaryTypeIndex::Dynamic:
        {
            const auto & dynamic_type = assert_cast<const DataTypeDynamic &>(*type);
            /// Maximum number of dynamic types is 255, we can write it as 1 byte.
            writeBinary(UInt8(dynamic_type.getMaxDynamicTypes()), buf);
            break;
        }
        case BinaryTypeIndex::AggregateFunction:
        {
            const auto & aggregate_function_type = assert_cast<const DataTypeAggregateFunction &>(*type);
            writeVarUInt(aggregate_function_type.getVersion(), buf);
            encodeAggregateFunction(aggregate_function_type.getFunctionName(), aggregate_function_type.getParameters(), aggregate_function_type.getArgumentsDataTypes(), buf);
            break;
        }
        case BinaryTypeIndex::SimpleAggregateFunction:
        {
            const auto & simple_aggregate_function_type = assert_cast<const DataTypeCustomSimpleAggregateFunction &>(*type->getCustomName());
            encodeAggregateFunction(simple_aggregate_function_type.getFunctionName(), simple_aggregate_function_type.getParameters(), simple_aggregate_function_type.getArgumentsDataTypes(), buf);
            break;
        }
        case BinaryTypeIndex::Nested:
        {
            const auto & nested_type = assert_cast<const DataTypeNestedCustomName &>(*type->getCustomName());
            const auto & elements = nested_type.getElements();
            const auto & names = nested_type.getNames();
            writeVarUInt(elements.size(), buf);
            for (size_t i = 0; i != elements.size(); ++i)
            {
                writeStringBinary(names[i], buf);
                encodeDataType(elements[i], buf);
            }
            break;
        }
        case BinaryTypeIndex::Custom:
        {
            const auto & type_name = type->getName();
            writeStringBinary(type_name, buf);
            break;
        }
        default:
            break;
    }
}

String encodeDataType(const DataTypePtr & type)
{
    WriteBufferFromOwnString buf;
    encodeDataType(type, buf);
    return buf.str();
}

DataTypePtr decodeDataType(ReadBuffer & buf)
{
    UInt8 type;
    readBinary(type, buf);
    switch (BinaryTypeIndex(type))
    {
        case BinaryTypeIndex::Nothing:
            return std::make_shared<DataTypeNothing>();
        case BinaryTypeIndex::UInt8:
            return std::make_shared<DataTypeUInt8>();
        case BinaryTypeIndex::Bool:
            return DataTypeFactory::instance().get("Bool");
        case BinaryTypeIndex::UInt16:
            return std::make_shared<DataTypeUInt16>();
        case BinaryTypeIndex::UInt32:
            return std::make_shared<DataTypeUInt32>();
        case BinaryTypeIndex::UInt64:
            return std::make_shared<DataTypeUInt64>();
        case BinaryTypeIndex::UInt128:
            return std::make_shared<DataTypeUInt128>();
        case BinaryTypeIndex::UInt256:
            return std::make_shared<DataTypeUInt256>();
        case BinaryTypeIndex::Int8:
            return std::make_shared<DataTypeInt8>();
        case BinaryTypeIndex::Int16:
            return std::make_shared<DataTypeInt16>();
        case BinaryTypeIndex::Int32:
            return std::make_shared<DataTypeInt32>();
        case BinaryTypeIndex::Int64:
            return std::make_shared<DataTypeInt64>();
        case BinaryTypeIndex::Int128:
            return std::make_shared<DataTypeInt128>();
        case BinaryTypeIndex::Int256:
            return std::make_shared<DataTypeInt256>();
        case BinaryTypeIndex::Float32:
            return std::make_shared<DataTypeFloat32>();
        case BinaryTypeIndex::Float64:
            return std::make_shared<DataTypeFloat64>();
        case BinaryTypeIndex::Date:
            return std::make_shared<DataTypeDate>();
        case BinaryTypeIndex::Date32:
            return std::make_shared<DataTypeDate32>();
        case BinaryTypeIndex::DateTimeUTC:
            return std::make_shared<DataTypeDateTime>();
        case BinaryTypeIndex::DateTimeWithTimezone:
        {
            String time_zone;
            readStringBinary(time_zone, buf);
            return std::make_shared<DataTypeDateTime>(time_zone);
        }
        case BinaryTypeIndex::DateTime64UTC:
        {
            UInt8 scale;
            readBinary(scale, buf);
            return std::make_shared<DataTypeDateTime64>(scale);
        }
        case BinaryTypeIndex::DateTime64WithTimezone:
        {
            UInt8 scale;
            readBinary(scale, buf);
            String time_zone;
            readStringBinary(time_zone, buf);
            return std::make_shared<DataTypeDateTime64>(scale, time_zone);
        }
        case BinaryTypeIndex::String:
            return std::make_shared<DataTypeString>();
        case BinaryTypeIndex::FixedString:
        {
            UInt64 size;
            readVarUInt(size, buf);
            return std::make_shared<DataTypeFixedString>(size);
        }
        case BinaryTypeIndex::Enum8:
            return decodeEnum<Int8>(buf);
        case BinaryTypeIndex::Enum16:
            return decodeEnum<Int16>(buf);
        case BinaryTypeIndex::Decimal32:
            return decodeDecimal<Decimal32>(buf);
        case BinaryTypeIndex::Decimal64:
            return decodeDecimal<Decimal64>(buf);
        case BinaryTypeIndex::Decimal128:
            return decodeDecimal<Decimal128>(buf);
        case BinaryTypeIndex::Decimal256:
            return decodeDecimal<Decimal256>(buf);
        case BinaryTypeIndex::UUID:
            return std::make_shared<DataTypeUUID>();
        case BinaryTypeIndex::Array:
            return std::make_shared<DataTypeArray>(decodeDataType(buf));
        case BinaryTypeIndex::NamedTuple:
        {
            size_t size;
            readVarUInt(size, buf);
            DataTypes elements;
            elements.reserve(size);
            Names names;
            names.reserve(size);
            for (size_t i = 0; i != size; ++i)
            {
                names.emplace_back();
                readStringBinary(names.back(), buf);
                elements.push_back(decodeDataType(buf));
            }

            return std::make_shared<DataTypeTuple>(elements, names);
        }
        case BinaryTypeIndex::UnnamedTuple:
        {
            size_t size;
            readVarUInt(size, buf);
            DataTypes elements;
            elements.reserve(size);
            for (size_t i = 0; i != size; ++i)
                elements.push_back(decodeDataType(buf));
            return std::make_shared<DataTypeTuple>(elements);
        }
        case BinaryTypeIndex::Set:
            return std::make_shared<DataTypeSet>();
        case BinaryTypeIndex::Interval:
        {
            UInt8 kind;
            readBinary(kind, buf);
            return std::make_shared<DataTypeInterval>(IntervalKind(IntervalKind::Kind(kind)));
        }
        case BinaryTypeIndex::Nullable:
            return std::make_shared<DataTypeNullable>(decodeDataType(buf));
        case BinaryTypeIndex::Function:
        {
            size_t arguments_size;
            readVarUInt(arguments_size, buf);
            DataTypes arguments;
            arguments.reserve(arguments_size);
            for (size_t i = 0; i != arguments_size; ++i)
                arguments.push_back(decodeDataType(buf));
            auto return_type = decodeDataType(buf);
            return std::make_shared<DataTypeFunction>(arguments, return_type);
        }
        case BinaryTypeIndex::LowCardinality:
            return std::make_shared<DataTypeLowCardinality>(decodeDataType(buf));
        case BinaryTypeIndex::Map:
        {
            auto key_type = decodeDataType(buf);
            auto value_type = decodeDataType(buf);
            return std::make_shared<DataTypeMap>(key_type, value_type);
        }
        case BinaryTypeIndex::Object:
        {
            bool has_nullable_subcolumns;
            readBinary(has_nullable_subcolumns, buf);
            String schema_format;
            readStringBinary(schema_format, buf);
            return std::make_shared<DataTypeObject>(schema_format, has_nullable_subcolumns);
        }
        case BinaryTypeIndex::IPv4:
            return std::make_shared<DataTypeIPv4>();
        case BinaryTypeIndex::IPv6:
            return std::make_shared<DataTypeIPv6>();
        case BinaryTypeIndex::Variant:
        {
            size_t size;
            readVarUInt(size, buf);
            DataTypes variants;
            variants.reserve(size);
            for (size_t i = 0; i != size; ++i)
                variants.push_back(decodeDataType(buf));
            return std::make_shared<DataTypeVariant>(variants);
        }
        case BinaryTypeIndex::Dynamic:
        {
            UInt8 max_dynamic_types;
            readBinary(max_dynamic_types, buf);
            return std::make_shared<DataTypeDynamic>(max_dynamic_types);
        }
        case BinaryTypeIndex::AggregateFunction:
        {
            size_t version;
            readVarUInt(version, buf);
            const auto & [function, parameters, arguments_types] = decodeAggregateFunction(buf);
            return std::make_shared<DataTypeAggregateFunction>(function, arguments_types, parameters, version);
        }
        case BinaryTypeIndex::SimpleAggregateFunction:
        {
            const auto & [function, parameters, arguments_types] = decodeAggregateFunction(buf);
            return createSimpleAggregateFunctionType(function, arguments_types, parameters);
        }
        case BinaryTypeIndex::Nested:
        {
            size_t size;
            readVarUInt(size, buf);
            Names names;
            names.reserve(size);
            DataTypes elements;
            elements.reserve(size);
            for (size_t i = 0; i != size; ++i)
            {
                names.emplace_back();
                readStringBinary(names.back(), buf);
                elements.push_back(decodeDataType(buf));
            }

            return createNested(elements, names);
        }
        case BinaryTypeIndex::Custom:
        {
            String type_name;
            readStringBinary(type_name, buf);
            return DataTypeFactory::instance().get(type_name);
        }
    }

    throw Exception(ErrorCodes::INCORRECT_DATA, "Unknown type code: {0:#04x}", UInt64(type));
}

DataTypePtr decodeDataType(const String & data)
{
    ReadBufferFromString buf(data);
    return decodeDataType(buf);
}

}
