// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_ENCODING_BASE64_HPP
#define COMMON_ENCODING_BASE64_HPP
#pragma once

#include <string>

class Base64
{
public:
    // Encodes the input string in base64.  Returns true if successful and false
    // otherwise.  The output string is only modified if successful.
    static bool Encode(const std::string& input, std::string* output);

    // Decodes the base64 input string.  Returns true if successful and false
    // otherwise.  The output string is only modified if successful.
    static bool Decode(const std::string& input, std::string* output);
};

#endif // COMMON_ENCODING_BASE64_HPP

