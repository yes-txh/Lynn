// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: Apr 20, 2011
// Description:

#undef NDEBUG
#include "common/base/int_cast.hpp"

#include <limits.h>
#include "common/base/global_initialize.hpp"
#include "common/base/stdint.h"
#include "gtest/gtest.h"

GLOBAL_INITIALIZE(int_cast_test)
{
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
}

TEST(IntCast, Value)
{
    EXPECT_EQ(127, int_cast<char>(127));
#if CHAR_MIN < 0
    EXPECT_EQ(-1, int_cast<char>(-1));
    EXPECT_EQ(-128, int_cast<char>(-128));
#endif
    EXPECT_EQ(127, int_cast<int8_t>(127));
    EXPECT_EQ(-128, int_cast<int8_t>(-128));
    EXPECT_EQ(255, int_cast<uint8_t>(255));

    EXPECT_EQ(32767, int_cast<int16_t>(32767));
    EXPECT_EQ(-32768, int_cast<int16_t>(-32768));
    EXPECT_EQ(65535, int_cast<uint16_t>(65535));

    EXPECT_EQ(2147483647, int_cast<int32_t>(2147483647));
    EXPECT_EQ(-2147483648LL, int_cast<int32_t>(-2147483648LL));
    EXPECT_EQ(4294967295U, int_cast<uint32_t>(4294967295U));

    EXPECT_EQ(9223372036854775807LL, int_cast<int64_t>(9223372036854775807LL));
    EXPECT_EQ(-9223372036854775807LL, int_cast<int64_t>(-9223372036854775807LL));
    EXPECT_EQ(18446744073709551615ULL, int_cast<uint64_t>(18446744073709551615ULL));
}

TEST(IntCast, Overflow)
{
#if CHAR_MIN < 0
    EXPECT_DEATH(int_cast<char>(128), "");
    EXPECT_DEATH(int_cast<char>(-129), "");
#else
    EXPECT_DEATH(int_cast<char>(256), "");
    EXPECT_DEATH(int_cast<char>(-1), "");
#endif
    EXPECT_DEATH(int_cast<int8_t>(128), "");
    EXPECT_DEATH(int_cast<int8_t>(-129), "");

    EXPECT_DEATH(int_cast<uint8_t>(256), "");
    EXPECT_DEATH(int_cast<uint8_t>(-1), "");

    EXPECT_DEATH(int_cast<int16_t>(32768), "");
    EXPECT_DEATH(int_cast<int16_t>(-32769), "");

    EXPECT_DEATH(int_cast<uint16_t>(65536), "");
    EXPECT_DEATH(int_cast<uint16_t>(-1), "");

    EXPECT_DEATH(int_cast<int32_t>(2147483648LL), "");
    EXPECT_DEATH(int_cast<int32_t>(-2147483649LL), "");

    EXPECT_DEATH(int_cast<uint32_t>(4294967296ULL), "");
    EXPECT_DEATH(int_cast<uint32_t>(-1), "");
    EXPECT_DEATH(int_cast<uint32_t>(-1LL), "");

    EXPECT_DEATH(int_cast<int64_t>(9223372036854775808ULL), "");

    // no way to test this: overflow
    // EXPECT_DEATH(int_cast<int64_t>(-9223372036854775809), "");
    // EXPECT_DEATH(int_cast<uint64_t>(18446744073709551616ULL), "");

    EXPECT_DEATH(int_cast<uint64_t>(-1), "");
    EXPECT_DEATH(int_cast<uint64_t>(-1LL), "");
}

