// Copyright 2011, Tencent Inc.
// All rights reserved.

/// @author XiaoDong Chen (donniechen@tencent.com)
///         CHEN Feng (phongchen@tencent.com)
/// @brief percent encoding process
/// @date Mar 31, 2011

#ifndef COMMON_ENCODING_PERCENT_HPP
#define COMMON_ENCODING_PERCENT_HPP

#include <ctype.h>
#include <string>
#include "common/base/stdint.h"

/// @brief percent encoding, majorly for url
/// @see http://en.wikipedia.org/wiki/Percent-encoding
struct PercentEncoding {
public:
    /// @brief 编码追加输出到 output
    static void EncodeAppend(const std::string &input, std::string* output);

    /// @brief 编码输出到 output
    static void EncodeTo(const std::string &input, std::string* output);

    /// @brief 编码自身，替换原来的内容
    static void Encode(std::string *str);

    /// @brief 返回编码后的结果
    static std::string Encode(const std::string &input);

    /// @brief 解码后，追加方式输出到 output 里
    static bool DecodeAppend(const std::string &input, std::string* output);

    /// @brief 解码后，输出到 output 里
    /// @return 是否成功
    static bool DecodeTo(const std::string &input, std::string* output);

    /// @brief 解码自身，替换原来的内容
    static bool Decode(std::string* str);
private:
    static uint8_t ToHex(const uint8_t x);
    static uint8_t FromHex(const uint8_t x);
    static bool IsValidEncodingChar(const uint8_t x);
};

#endif // COMMON_ENCODING_PERCENT_HPP
