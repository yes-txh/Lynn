// Copyright (c) 2011, Tencent.com
// All rights reserved.

/// @file string_number_test.cpp
/// @brief string_number_test
/// @date  03/31/2011 10:42:51 AM
/// @author CHEN Feng <phongchen@tencent.com>

#include "common/base/string/string_number.hpp"
#include <math.h>
#include "gtest/gtest.h"

TEST(StringNumber, IsNaN)
{
    float f = 1.000;
    ASSERT_FALSE(IsNaN(f));
    f = 1.223e+20;
    ASSERT_FALSE(IsNaN(f));
#ifdef __GNUC__
    f = 1.0 / 0.0;
    ASSERT_FALSE(IsNaN(f));
#endif
    f = sqrt((double)-1);
    ASSERT_TRUE(IsNaN(f));
}

TEST(StringNumber, IntegerToStringBuffer)
{
    char buffer[1024];
    int32_t n1 = INT_MAX;
    int32_t n2 = -INT_MAX;
    int32_t n3 = 0;
    int32_t n4 = 100000;
    uint32_t n5 = 3147483647U;
    int32_t n6 = -123456789;

    int64_t s1 = LLONG_MAX;
    int64_t s2 = INT_MAX;
    int64_t s3 = 0;
    int64_t s4 = 1234567890123LL;
    int64_t s5 = 1000000000000LL;
    int64_t s6 = -1234567890034500LL;
    int64_t s7 = LLONG_MIN;

    ASSERT_STREQ("2147483647", IntegerToString(n1, buffer));
    ASSERT_STREQ("-2147483647", IntegerToString(n2, buffer));
    ASSERT_STREQ("0", IntegerToString(n3, buffer));
    ASSERT_STREQ("100000", IntegerToString(n4, buffer));
    ASSERT_STREQ("3147483647", IntegerToString(n5, buffer));
    ASSERT_STREQ("-123456789", IntegerToString(n6, buffer));

    ASSERT_STREQ("9223372036854775807", IntegerToString(s1, buffer));
    ASSERT_STREQ("2147483647", IntegerToString(s2, buffer));
    ASSERT_STREQ("0", IntegerToString(s3, buffer));
    ASSERT_STREQ("1234567890123", IntegerToString(s4, buffer));
    ASSERT_STREQ("1000000000000", IntegerToString(s5, buffer));
    ASSERT_STREQ("-1234567890034500", IntegerToString(s6, buffer));
    ASSERT_STREQ("-9223372036854775808", IntegerToString(s7, buffer));
}

TEST(StringNumber, IntegerToString)
{
    int32_t n1 = INT_MAX;
    int32_t n2 = -INT_MAX;
    int32_t n3 = 0;
    int32_t n4 = 100000;
    uint32_t n5 = 3147483647U;
    int32_t n6 = -123456789;

    int64_t s1 = LLONG_MAX;
    int64_t s2 = INT_MAX;
    int64_t s3 = 0;
    int64_t s4 = 1234567890123LL;
    int64_t s5 = 1000000000000LL;
    int64_t s6 = -1234567890034500LL;
    int64_t s7 = LLONG_MIN;

    ASSERT_EQ("2147483647", IntegerToString(n1));
    ASSERT_EQ("-2147483647", IntegerToString(n2));
    ASSERT_EQ("0", IntegerToString(n3));
    ASSERT_EQ("100000", IntegerToString(n4));
    ASSERT_EQ("3147483647", IntegerToString(n5));
    ASSERT_EQ("-123456789", IntegerToString(n6));

    ASSERT_EQ("9223372036854775807", IntegerToString(s1));
    ASSERT_EQ("2147483647", IntegerToString(s2));
    ASSERT_EQ("0", IntegerToString(s3));
    ASSERT_EQ("1234567890123", IntegerToString(s4));
    ASSERT_EQ("1000000000000", IntegerToString(s5));
    ASSERT_EQ("-1234567890034500", IntegerToString(s6));
    ASSERT_EQ("-9223372036854775808", IntegerToString(s7));
}

TEST(StringNumber, UIntToHexString)
{
    EXPECT_EQ("9527", UInt16ToHexString(0x9527));
    EXPECT_EQ("95279527", UInt32ToHexString(0x95279527));
    EXPECT_EQ("9527952795279527", UInt64ToHexString(0x9527952795279527ULL));
}

TEST(StringNumber, StringToNumber)
{
    int16_t i16;
    int32_t i32;
    int64_t i64;
    ASSERT_FALSE(StringToNumber("223372036854775807", &i32));
    ASSERT_TRUE(StringToNumber("223372036854775807", &i64));
    ASSERT_EQ(i64, 223372036854775807LL);
    ASSERT_FALSE(StringToNumber("1147483647", &i16));
    ASSERT_TRUE(StringToNumber("1147483647", &i32));
    ASSERT_TRUE(StringToNumber("1147483647", &i64));
    ASSERT_EQ(i32, 1147483647);
    ASSERT_EQ(i64, 1147483647);

    uint32_t u32;
    ASSERT_TRUE(StringToNumber("1147483647", &u32));

    char buffer[1024];
    double d = 1.0003;

    ASSERT_STREQ(DoubleToString(d, buffer), "1.0003");
    d = std::numeric_limits<double>::infinity();
    ASSERT_STREQ(DoubleToString(d, buffer), "inf");
    d = -std::numeric_limits<double>::infinity();
    ASSERT_STREQ(DoubleToString(d, buffer), "-inf");
#ifdef __GNUC__ // divided by zero is not allowed in msvc
    d = 0.0f / 0.0f;
    ASSERT_STREQ(DoubleToString(d, buffer), "nan");
#endif

    float f = 1e+22;
    ASSERT_STREQ(FloatToString(f, buffer), "1e+22");
    f = 0.000325;
    ASSERT_STREQ(FloatToString(f, buffer), "0.000325");
    f = std::numeric_limits<double>::infinity();
    ASSERT_STREQ(FloatToString(f, buffer), "inf");
    f = -std::numeric_limits<double>::infinity();
    ASSERT_STREQ(FloatToString(f, buffer), "-inf");

#ifdef __GNUC__ // divided by zero is not allowed in msvc
    f = 0.0f / 0.0f;
    ASSERT_STREQ(FloatToString(f, buffer), "nan");

    f = 1 / 0.0f;
    ASSERT_STREQ(FloatToString(f, buffer), "inf");
#endif

    f = -std::numeric_limits<float>::infinity();
    ASSERT_STREQ(FloatToString(f, buffer), "-inf");


    uint32_t i = 255;
    ASSERT_STREQ(UInt32ToHexString(i, buffer), "000000ff");

    std::string str = "1110.32505QQ";
    char* endptr;
    ASSERT_TRUE(ParseNumber(str.c_str(), &d, &endptr));
    ASSERT_FALSE(StringToNumber(str.c_str(), &d));
    ASSERT_TRUE(d == 1110.32505);
    ASSERT_TRUE(ParseNumber(str.c_str(), &f, &endptr));
    ASSERT_FALSE(StringToNumber(str.c_str(), &f));
    ASSERT_TRUE(f == 1110.32505f);
    ASSERT_TRUE(ParseNumber(str.c_str(), &i, &endptr));
    ASSERT_FALSE(StringToNumber(str.c_str(), &i));
    ASSERT_EQ(i, (uint32_t)1110);

    str = "1110.32505";
    d = 0;
    f = 0;
    i = 0;
    ASSERT_TRUE(StringToNumber(str.c_str(), &d));
    ASSERT_TRUE(d == 1110.32505);
    ASSERT_TRUE(StringToNumber(str.c_str(), &f));
    ASSERT_TRUE(f == 1110.32505f);
    ASSERT_FALSE(StringToNumber(str.c_str(), &i));
    str = "-1110";
    int32_t x;
    ASSERT_TRUE(StringToNumber(str.c_str(), &x));
    ASSERT_EQ(x, -1110);
}

class StringToNumberPerformanceTest : public testing::Test
{
protected:
    static const unsigned int kNumber = 0x42576010U;
    static const char kString[];
private:
    void SetUp()
    {
        unsigned int n;
        sscanf(kString, "%x", &n);
        ASSERT_EQ(0x42576010U, n);
        ASSERT_TRUE(StringToNumber(kString, &n));
        ASSERT_EQ(0x42576010U, n);
    }
};

const char StringToNumberPerformanceTest::kString[] = "0x42576010";

TEST_F(StringToNumberPerformanceTest, SScanfPerformance)
{
    for (int i = 0; i < 1000000; i++)
    {
        unsigned int n;
        sscanf(kString, "%x", &n);
    }
}

TEST_F(StringToNumberPerformanceTest, StringToNumberPerformance)
{
    for (int i = 0; i < 1000000; i++)
    {
        unsigned int n;
        StringToNumber(kString, &n);
    }
}

TEST(StringNumber, NumberToStringPerformance)
{
    double d = 1110.32505f;
    for (size_t i = 0; i < 100000; i++)
    {
        DoubleToString(d);
    }
}


