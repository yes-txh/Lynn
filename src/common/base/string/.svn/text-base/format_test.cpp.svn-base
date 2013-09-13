// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 04/30/11
// Description: test string format functions

#include "common/base/string/format.hpp"
#include "gtest/gtest.h"

TEST(String, Printf)
{
    long unsigned int lu = 99;
    ASSERT_EQ(StringFormat("sx%d%s%lu\n", 100, "hehe,", lu), "sx100hehe,99\n");

    std::string str;
    size_t length = StringFormatTo(&str, "sx%d%s%lu\n", 100, "hehe,", lu);
    ASSERT_EQ(str, "sx100hehe,99\n");
    ASSERT_EQ(str.length(), length);

    size_t append_length = StringFormatAppend(&str, "sx%d%s%lu\n", 100, "hehe,", lu);
    ASSERT_EQ(str, "sx100hehe,99\nsx100hehe,99\n");
    ASSERT_EQ(append_length, str.length() - length);
}


