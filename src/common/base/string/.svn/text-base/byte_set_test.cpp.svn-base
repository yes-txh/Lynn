// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/11/11

#include "common/base/string/byte_set.hpp"
#include "gtest/gtest.h"

TEST(ByteSet, Empty)
{
    ByteSet bs;
    EXPECT_FALSE(bs.Find('A'));
    bs.Insert('A');
    EXPECT_TRUE(bs.Find('A'));
}
