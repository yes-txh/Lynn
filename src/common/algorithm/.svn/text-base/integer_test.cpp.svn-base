#include "common/algorithm/integer.hpp"
#include <limits.h>
#include <gtest/gtest.h>

TEST(Int, ConstLog2)
{
    unsigned int total = 0;
    const unsigned int STEP = 128;
    for (unsigned int i = 0; i < UINT_MAX - STEP; i += STEP)
        total = ConstLog2(i);
    volatile unsigned int b = total;
    (void) b;
}

#ifdef __GNUC__
TEST(Int, AsmLog2)
{
    unsigned int total = 0;
    const unsigned int STEP = 128;
    for (unsigned int i = 0; i < UINT_MAX - STEP; i += STEP)
    {
        total = AsmLog2(i);
    }
    volatile unsigned int b = total;
    (void) b;
}
#endif

TEST(Int, UpperPowerOf2)
{
    EXPECT_EQ(1U, UpperPowerOf2(0));
    EXPECT_EQ(1U, UpperPowerOf2(1));
    EXPECT_EQ(4U, UpperPowerOf2(3));
    EXPECT_EQ(4U, UpperPowerOf2(4));
    EXPECT_EQ(8U, UpperPowerOf2(5));
    EXPECT_EQ(16U, UpperPowerOf2(9));
    EXPECT_EQ(32U, UpperPowerOf2(30));
}

TEST(Int, PopCount)
{
    EXPECT_EQ(1, PopCount(1));
    EXPECT_EQ(1, PopCount(2));
    EXPECT_EQ(1, PopCount(4));
    EXPECT_EQ(2, PopCount(3));
    EXPECT_EQ(3, PopCount(7));
    EXPECT_EQ(4, PopCount(15));
    EXPECT_EQ(8, PopCount(255));
}
