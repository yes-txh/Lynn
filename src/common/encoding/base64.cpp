// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/encoding/base64.hpp"
#include "common/encoding/modp_b64/modp_b64.h"

bool Base64::Encode(const std::string& input, std::string* output)
{
    std::string temp;
    temp.resize(modp_b64_encode_len(input.size()));  // makes room for null byte

    // null terminates result since result is base64 text!
    int input_size = static_cast<int>(input.size());
    int output_size= modp_b64_encode(&(temp[0]), input.data(), input_size);
    if (output_size < 0)
        return false;

    temp.resize(output_size);  // strips off null byte
    output->swap(temp);
    return true;
}

bool Base64::Decode(const std::string& input, std::string* output)
{
    std::string temp;
    temp.resize(modp_b64_decode_len(input.size()));

    // does not null terminate result since result is binary data!
    int input_size = static_cast<int>(input.size());
    int output_size = modp_b64_decode(&(temp[0]), input.data(), input_size);
    if (output_size < 0)
        return false;

    temp.resize(output_size);
    output->swap(temp);
    return true;
}

