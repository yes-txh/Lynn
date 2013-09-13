// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

///////////////////////////////////////////////////////////////////////////
/// @brief percent encoding test
/// @author phongchen@tencent
/// @date 2011-03-29
///////////////////////////////////////////////////////////////////////////

#include "common/encoding/percent.hpp"
#include "common/base/array_size.h"
#include "common/base/string/string_algorithm.hpp"
#include "common/encoding/percent_test_gbk.cpp"
#include "common/encoding/percent_test_utf8.cpp"
#include "gtest/gtest.h"

TEST(PercentEncoding, Space)
{
    EXPECT_EQ("percent+encoding", PercentEncoding::Encode("percent encoding"));
    std::string decoded;
    EXPECT_TRUE(PercentEncoding::DecodeTo("percent+encoding", &decoded));
    EXPECT_EQ("percent encoding", decoded);
}

TEST(PercentEncoding, Inplace)
{
    std::string str = kTigerUtf8;
    PercentEncoding::Encode(&str);
    ASSERT_EQ(kTigerUtf8Encoded, str);

    ASSERT_TRUE(PercentEncoding::Decode(&str));
    ASSERT_EQ(kTigerUtf8, str);
}

TEST(PercentEncoding, Case)
{
    std::string encoded = kTigerUtf8Encoded;
    LowerString(&encoded);
    std::string decoded;
    EXPECT_TRUE(PercentEncoding::DecodeTo(encoded, &decoded));
    EXPECT_EQ(kTigerUtf8, decoded);
}

TEST(PercentEncoding, DecodeError)
{
    std::string encoded(kTigerUtf8Encoded, ARRAY_SIZE(kTigerUtf8Encoded) - 2);
    std::string decoded;
    EXPECT_FALSE(PercentEncoding::DecodeTo(encoded, &decoded));
}

TEST(PercentEncoding, Chinese)
{
    EXPECT_EQ(kTigerUtf8Encoded, PercentEncoding::Encode(kTigerUtf8));

    std::string decoded;
    EXPECT_TRUE(PercentEncoding::DecodeTo(kTigerUtf8Encoded, &decoded));
    EXPECT_EQ(kTigerUtf8, decoded);

    EXPECT_EQ(kTigerGBKEncoded, PercentEncoding::Encode(kTigerGBK));
    EXPECT_TRUE(PercentEncoding::DecodeTo(kTigerGBKEncoded, &decoded));
    EXPECT_EQ(kTigerGBK, decoded);
}

TEST(PercentEncoding, Url)
{
    EXPECT_EQ("http%3A%2F%2Fwww.baidu.com%2Fs%3Fbs%3D",
              PercentEncoding::Encode("http://www.baidu.com/s?bs="));
    std::string decoded;
    EXPECT_TRUE(PercentEncoding::DecodeTo("http://www.baidu.com/s?bs=", &decoded));
    EXPECT_EQ("http://www.baidu.com/s?bs=", decoded);
}

// verified by baidu & google
const char kSpecial[] = "&.<;/.\\~!@#$%^*()_|;':\"[]{}|,.<>/?+-=";
const char kSpecialEncoded[] =
    "%26.%3C%3B%2F.\\~!%40%23%24%25^*%28%29_|%3B%27%3A%22[]{}|%2C.%3C%3E%2F%3F%2B-%3D";

TEST(PercentEncoding, Special)
{
    EXPECT_EQ(kSpecialEncoded, PercentEncoding::Encode(kSpecial));
    std::string decoded;
    EXPECT_TRUE(PercentEncoding::DecodeTo(kSpecialEncoded, &decoded));
    EXPECT_EQ(kSpecial, decoded);
}
