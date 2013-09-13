// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_ENCODING_HEX_HPP
#define COMMON_ENCODING_HEX_HPP

/// @file
/// @author Chen Feng <phongchen@tencent.com>
/// @date Jan 12, 2011

#include <iterator>
#include <string>

/// @brief execute hex encoding
/// @tparam ForwardIterator forward iterator
/// @tparam OutputIterator output iterator
/// @param first start of encoding range
/// @param last end of encoding range
/// @param uppercase whether yield upper case result
template <typename ForwardIterator, typename OutputIterator>
void HexEncode(
    ForwardIterator first,
    ForwardIterator last,
    OutputIterator output,
    bool uppercase = false
    )
{
    const char* hex_digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    while (first != last)
    {
        unsigned char ch = *first;
        *output++ = hex_digits[ch >> 4];
        *output++ = hex_digits[ch & 0x0F];
        ++first;
    }
}

/// @brief hex encoding, append result to string
/// @tparam Container any STL compatible container that support push_back
/// @param data data to be encoded
/// @param size data size
/// @param output buffer to output
/// @param uppercase whether yield upper case result
/// @return *output as Container&
template <typename Container>
Container& HexEncodeAppend(
    const void* data, size_t size,
    Container* output,
    bool uppercase = false
    )
{
    const unsigned char* p = static_cast<const unsigned char*>(data);
    HexEncode(p, p + size, std::back_inserter(*output), uppercase);
    return *output;
}

/// @brief hex encoding, output result to string(overwrite)
/// @tparam Container any STL compatible container that support push_back
/// @param data data to be encoded
/// @param size data size
/// @param output buffer to output
/// @param uppercase whether yield upper case result
/// @return *output as Container&
template <typename Container>
Container& HexEncodeTo(
    const void* data, size_t size,
    Container* output,
    bool uppercase = false
    )
{
    output->clear();
    return HexEncodeAppend(data, size, output, uppercase);
}

/// @brief hex encoding, and return result as string
/// @param data data to be encoded
/// @param size data size
/// @param uppercase whether yield upper case result
/// @return encoded result as string
inline std::string HexEncodeString(
    const void* data, size_t size,
    bool uppercase = false
    )
{
    std::string str;
    HexEncodeTo(data, size, &str, uppercase);
    return str;
}

/// @brief hex encoding to buffer
/// @param data data to be encoded
/// @param size data size
/// @param output output buffer
/// @param uppercase whether yield upper case result
/// @return output buffer
/// @note output buffer size must be large enough, at lease 2 * size + 1
inline char* HexEncodeToBuffer(
    const void* data, size_t size,
    char* output,
    bool uppercase = false
    )
{
    const unsigned char* p = static_cast<const unsigned char*>(data);
    HexEncode(p, p + size, output, uppercase);
    output[2 * size] = '\0';
    return output;
}


#endif // COMMON_ENCODING_HEX_HPP
