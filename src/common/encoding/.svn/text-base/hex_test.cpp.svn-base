// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/base/array_size.h"
#include "common/encoding/hex.hpp"
#include "gtest/gtest.h"

const char g_data[] = { '\x12', '\x34', '\x56', '\xFF' };

TEST(HexEncoding, Generic)
{
    char result[2 * ARRAY_SIZE(g_data) + 1] = {};

    char end_char = result[2 * ARRAY_SIZE(g_data)];
    HexEncode(g_data, g_data + ARRAY_SIZE(g_data), result, true);
    EXPECT_EQ(end_char, result[2 * ARRAY_SIZE(g_data)]);

    result[2 * ARRAY_SIZE(g_data)] = '\0';
    EXPECT_STREQ("123456FF", result);
}

static bool IsEqual(const std::string& str, const std::vector<char>& v)
{
    return str.size() == v.size() &&
        std::equal(str.begin(), str.end(), v.begin());
}

TEST(HexEncoding, Buffer)
{
    char result[2 * ARRAY_SIZE(g_data) + 1];
    EXPECT_STREQ("123456FF", HexEncodeToBuffer(g_data, ARRAY_SIZE(g_data), result, true));
}

TEST(HexEncoding, HexEncodeAppend)
{
    std::string result = "0x";
    EXPECT_EQ("0x123456FF", HexEncodeAppend(g_data, ARRAY_SIZE(g_data), &result, true));

    std::vector<char> vector_result;
    vector_result.push_back('0');
    vector_result.push_back('x');
    HexEncodeAppend(g_data, ARRAY_SIZE(g_data), &vector_result, true);
    EXPECT_TRUE(IsEqual(result, vector_result));
}

TEST(HexEncoding, HexEncodeTo)
{
    std::string result = "0x";
    EXPECT_EQ("123456FF", HexEncodeTo(g_data, ARRAY_SIZE(g_data), &result, true));

    std::vector<char> vector_result;
    HexEncodeTo(g_data, ARRAY_SIZE(g_data), &vector_result, true);
    EXPECT_TRUE(IsEqual(result, vector_result));
}

TEST(HexEncoding, Case)
{
    EXPECT_EQ("123456ff", HexEncodeString(g_data, ARRAY_SIZE(g_data), false));
}

