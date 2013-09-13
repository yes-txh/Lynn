// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/base/array_size.h"
#include <gtest/gtest.h>

TEST(ArraySize, Array)
{
    char a[2];
    ASSERT_EQ(sizeof(a), ARRAY_SIZE(a));
}

TEST(ArraySize, ConstArray)
{
    const char a[2] = {};
    ASSERT_EQ(sizeof(a), ARRAY_SIZE(a));
}

TEST(ArraySize, EmptyArray)
{
    char a[] = {};
    ASSERT_EQ(sizeof(a), ARRAY_SIZE(a));
}

TEST(ArraySize, Scalable)
{
    // uncomment to test compile checking
    // char a; ASSERT_EQ(sizeof(a), ARRAY_SIZE(a));
}

TEST(ArraySize, Pointer)
{
    // uncomment to test compile checking
    // char* p; ASSERT_EQ(sizeof(p), ARRAY_SIZE(p));
}
