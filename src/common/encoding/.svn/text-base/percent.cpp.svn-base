// Copyright (c) 2011, Tencent.com
// All rights reserved.

/// @file percent.cpp
/// @brief percent encoding implementation
/// @date  03/31/2011 03:57:46 PM
/// @author CHEN Feng <phongchen@tencent.com>

#include "common/encoding/percent.hpp"
#include <iso646.h>
#include "common/encoding/ascii.hpp"

///////////////////////////////////////////////////////////////////////////
// helpers
///////////////////////////////////////////////////////////////////////////

inline uint8_t PercentEncoding::ToHex(const uint8_t x)
{
    return "0123456789ABCDEF"[x];
}

inline uint8_t PercentEncoding::FromHex(const uint8_t x)
{
    uint8_t X = Ascii::ToUpper(x);
    return Ascii::IsDigit(X) ? X-'0' : X-'A'+10;
}

inline bool PercentEncoding::IsValidEncodingChar(const uint8_t x)
{
    uint8_t X = Ascii::ToUpper(x);
    if (Ascii::IsDigit(X) || (X >= 'A' && X <= 'F'))
        return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////
// encode
///////////////////////////////////////////////////////////////////////////

void PercentEncoding::EncodeAppend(const std::string &input, std::string* output)
{
    for (size_t ix = 0; ix < input.size(); ix++) {
        if (Ascii::IsAlphaNumber((uint8_t)input[ix])
            || input[ix] == '-'
            || input[ix] == '_'
            || input[ix] == '.'
            || input[ix] == '['
            || input[ix] == ']'
            || input[ix] == '{'
            || input[ix] == '}'
            || input[ix] == '|'
            || input[ix] == '*'
            || input[ix] == '^'
            || input[ix] == '~'
            || input[ix] == '!'
            || input[ix] == '\\') {
            output->push_back(input[ix]);
        } else if (input[ix] == ' ') {
            // 貌似把空格编码成%20或者+都可以，这里编成+号
            output->push_back('+');
        } else {
            output->push_back('%');
            output->push_back(ToHex((uint8_t)input[ix] >> 4));
            output->push_back(ToHex((uint8_t)input[ix] % 16));
        }
    }
}

void PercentEncoding::EncodeTo(const std::string &input, std::string* output)
{
    output->clear();
    EncodeAppend(input, output);
}

std::string PercentEncoding::Encode(const std::string &input) {
    std::string result;
    EncodeAppend(input, &result);
    return result;
}

void PercentEncoding::Encode(std::string *str)
{
    std::string tmp;
    EncodeAppend(*str, &tmp);
    std::swap(*str, tmp);
}

///////////////////////////////////////////////////////////////////////////////
// decode
///////////////////////////////////////////////////////////////////////////////

bool PercentEncoding::DecodeAppend(const std::string &input, std::string* output)
{
    for (size_t i = 0; i < input.size(); ++i) {
        uint8_t ch = 0;
        if (input[i] == '%') {
            if (i + 2 > input.size()) {
                // 后面的数据不完整了，返回吧
                return false;
            }

            if (not IsValidEncodingChar(input[i+1]) || not IsValidEncodingChar(input[i+2]))
                return false;

            ch = (FromHex(input[i+1])<<4);
            ch |= FromHex(input[i+2]);
            i += 2;
        } else if (input[i] == '+') {
            ch = ' ';
        } else {
            ch = input[i];
        }
        *output += static_cast<char>(ch);
    }
    return true;
}

bool PercentEncoding::DecodeTo(const std::string &input, std::string* output)
{
    output->clear();
    return DecodeAppend(input, output);
}

bool PercentEncoding::Decode(std::string *str)
{
    std::string& s = *str;
    size_t write_pos = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        uint8_t ch = 0;
        if (s[i] == '%') {
            if (i + 2 > s.size()) {
                // 后面的数据不完整了，返回吧
                return false;
            }

            if (not IsValidEncodingChar(s[i+1]) || not IsValidEncodingChar(s[i+2]))
                return false;

            ch = (FromHex(s[i+1])<<4);
            ch |= FromHex(s[i+2]);
            i += 2;
        } else if (s[i] == '+') {
            ch = ' ';
        } else {
            ch = s[i];
        }
        s[write_pos++] = static_cast<char>(ch);
    }
    s.resize(write_pos);
    return true;
}


