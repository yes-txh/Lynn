// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/encoding/base64.hpp"
#include <gtest/gtest.h>

const std::string kText = "hello world";
const std::string kBase64Text = "aGVsbG8gd29ybGQ=";

TEST(Base64Test, Encode)
{
    std::string result;
    EXPECT_TRUE(Base64::Encode(kText, &result));
    EXPECT_EQ(kBase64Text, result);
}

TEST(Base64Test, Decode)
{
    std::string result;
    EXPECT_TRUE(Base64::Decode(kBase64Text, &result));
    EXPECT_EQ(kText, result);
}

