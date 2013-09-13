// Copyright (c) 2008, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_SERIALIZE_SERIALIZE_HPP
#define COMMON_BASE_SERIALIZE_SERIALIZE_HPP

#include <assert.h>

#include "common/base/serialize/type.hpp"
#include "common/base/serialize/stl.hpp"
#include "common/base/serialize/stream.hpp"
#include "common/base/serialize/binary_codec.hpp"
#include "common/base/serialize/xml_codec.hpp"
#include "common/base/serialize/json_codec.hpp"

// GLOBAL_NOLINT(runtime/int)

namespace Serialize
{

/// Binary encode to vector<char>
template <typename T, typename Allocator>
void BinaryEncode(const T& value, std::vector<char, Allocator>& buffer)
{
    VectorOutputStream stream(buffer);
    BinaryEncoder<VectorOutputStream> encoder(stream);
    encoder << value;
}

/// Binary encode append to vector<char>
template <typename T, typename Allocator>
void BinaryEncodeAppend(const T& value, std::vector<char, Allocator>& buffer)
{
    VectorOutputStream stream(buffer, buffer.size());
    BinaryEncoder<VectorOutputStream> encoder(stream);
    encoder << value;
}

/// Binary encode to std::string
template <typename T>
void BinaryEncode(const T& value, std::string& buffer)
{
    StringOutputStream stream(buffer);
    BinaryEncoder<StringOutputStream> encoder(stream);
    encoder << value;
}

/// Binary encode append to std::string
template <typename T>
void BinaryEncodeAppend(const T& value, std::string& buffer)
{
    StringOutputStream stream(buffer, buffer.size());
    BinaryEncoder<StringOutputStream> encoder(stream);
    encoder << value;
}

/// Binary encode to memory buffer
template <typename T>
size_t BinaryEncode(const T& value, void* buffer, size_t size)
{
    BufferOutputStream stream(buffer, size);
    BinaryEncoder<BufferOutputStream> encoder(stream);
    encoder << value;
    return stream.Tell();
}

/// Binary encode to memory buffer without buffer overflow checking
template <typename T>
size_t BinaryEncodeUnchecked(const T& value, void* buffer)
{
    UncheckedBufferOutputStream stream(buffer);
    BinaryEncoder<UncheckedBufferOutputStream> encoder(stream);
    encoder << value;
    return stream.Tell();
}

/// Binary encode append to FILE
template <typename T>
long long BinaryEncode(const T& value, FILE* fp)
{
    StdoutStream stream(fp);
    BinaryEncoder<StdoutStream> encoder(stream);
    encoder << value;
    return stream.Tell();
}

/// Get binary encoded size
template <typename T>
size_t BinaryEncodedSize(const T& value)
{
    NullOutputStream stream;
    BinaryEncoder<NullOutputStream> encoder(stream);
    encoder << value;
    return static_cast<size_t>(stream.Tell());
}

/// binary decode from memory buffer
/// @return decoded buffer size
template <typename T>
size_t BinaryDecode(
    const void* buffer,
    size_t size,
    T& value,
    ErrorStack* error_stack = NULL
    )
{
    BufferInputStream stream(buffer, size);
    BinaryDecoder<BufferInputStream> decoder(stream);
    if (Decode(decoder, value))
        return stream.Tell();
    if (error_stack)
        *error_stack = decoder.ErrorStack;
    return 0;
}

/// binary decode from vector
/// @param where to decode
/// @return decoded buffer size
template <typename T, typename Allocator>
size_t BinaryDecode(
    std::vector<char, Allocator>& vector,
    size_t skip,
    T& value,
    ErrorStack* error_stack = NULL
    )
{
    assert(skip < vector.size());
    return BinaryDecode(value, &vector[skip], vector.size() - skip, error_stack);
}

/// binary decode from vector
/// @return decoded buffer size
template <typename T, typename Allocator>
size_t BinaryDecode(
    std::vector<char, Allocator>& vector,
    T& value,
    ErrorStack* error_stack = NULL
    )
{
    return BinaryDecode(&vector[0], vector.size(), value, error_stack);
}

/// binary decode from string
/// @param where to decode
/// @return decoded buffer size
template <typename T>
size_t BinaryDecode(
    std::string& string,
    size_t skip,
    T& value,
    ErrorStack* error_stack = NULL
    )
{
    assert(skip < string.size());
    return BinaryDecode(value, &string[skip], string.size() - skip, error_stack);
}

/// binary decode from string
/// @return decoded buffer size
template <typename T>
size_t BinaryDecode(
    std::string& string,
    T& value,
    ErrorStack* error_stack = NULL
    )
{
    return BinaryDecode(&string[0], string.size(), value, error_stack);
}

template <typename T>
long long BinaryDecode(
    FILE* fp,
    T& value,
    ErrorStack* error_stack = NULL
    )
{
    StdinStream stream(fp);
    BinaryDecoder<StdinStream> decoder(stream);
    if (Decode(decoder, value))
        return stream.Tell();
    if (error_stack)
        *error_stack = decoder.ErrorStack;
    return 0;
}

/// Dump to FILE as JSOM format
/// @param indent whether indent
/// @param comment where generate comment
template <typename T>
void JsonDump(const T& value, FILE* fp, bool indent = true, bool comment = true)
{
    StdoutStream stream(fp);
    JsonEncoder<StdoutStream> encoder(stream, indent, comment);
    encoder << value;

    // encoder doesn't output ending \n, add it
    if (indent)
        fputc('\n', fp);
}

/// Dump to FILE as JSOM format
/// @param indent whether indent
/// @param comment where generate comment
template <typename T>
void JsonDump(const T& value, std::ostream& os, bool indent = true, bool comment = true)
{
    CxxOutputStream stream(os);
    JsonEncoder<CxxOutputStream> encoder(stream, indent, comment);
    encoder << value;

    // encoder doesn't output ending \n, add it
    if (indent)
        os.put('\n');
}

/// Get XML encoded size
template <typename T>
size_t XmlEncodedSize(const T& value)
{
    NullOutputStream stream;
    XmlEncoder<NullOutputStream> encoder(stream);
    encoder << value;
    return stream.Tell();
}

/// XML dump to FILE
template <typename T>
void XmlDump(const T& value, FILE* fp)
{
    StdoutStream stream(fp);
    XmlEncoder<StdoutStream> encoder(stream);
    encoder << value;
}

/// XML dump to buffer
template <typename T>
size_t XmlDump(const T& value, void* buffer, size_t size)
{
    BufferOutputStream stream(buffer, size);
    XmlEncoder<BufferOutputStream> encoder(stream);
    encoder << value;
    return stream.Tell();
}

/// XML dump to vector<char>
template <typename T, typename Allocator>
void XmlDump(const T& value, std::vector<char, Allocator>& buffer)
{
    VectorOutputStream stream(buffer);
    XmlEncoder<VectorOutputStream> encoder(stream);
    encoder << value;
}

}

#endif // COMMON_BASE_SERIALIZE_SERIALIZE_HPP
