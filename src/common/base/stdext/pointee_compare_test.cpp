#include <algorithm>
#include "common/base/stdext/pointee_compare.hpp"
#include "common/base/array_size.h"
#include <gtest/gtest.h>

TEST(ComparePointee, Less)
{
    int a = 1, b = 2;
    int *p = &a;
    int *q = &b;
    EXPECT_TRUE(stdext::pointee_less<int>()(p, q));
    EXPECT_FALSE(stdext::pointee_less<int>()(q, p));
}

TEST(ComparePointee, LessEqual)
{
    int a = 1, b = 2;
    int *p = &a;
    int *q = &b;
    EXPECT_TRUE(stdext::pointee_less_equal<int>()(p, q));
    EXPECT_FALSE(stdext::pointee_less_equal<int>()(q, p));
    EXPECT_TRUE(stdext::pointee_less_equal<int>()(p, p));
}

TEST(ComparePointee, Greater)
{
    int a = 1, b = 2;
    int *p = &a;
    int *q = &b;
    EXPECT_FALSE(stdext::pointee_greater<int>()(p, q));
    EXPECT_TRUE(stdext::pointee_greater<int>()(q, p));
}

TEST(ComparePointee, GreaterEqual)
{
    int a = 1, b = 2;
    int *p = &a;
    int *q = &b;
    EXPECT_FALSE(stdext::pointee_greater_equal<int>()(p, q));
    EXPECT_TRUE(stdext::pointee_greater_equal<int>()(q, p));
    EXPECT_TRUE(stdext::pointee_greater_equal<int>()(p, p));
}

TEST(ComparePointee, EqualTo)
{
    int a = 1, b = 2;
    int *p = &a;
    int *q = &b;
    EXPECT_FALSE(stdext::pointee_equal_to<int>()(p, q));
    EXPECT_TRUE(stdext::pointee_equal_to<int>()(p, p));
}

TEST(ComparePointee, NotEqualTo)
{
    int a = 1, b = 2;
    int *p = &a;
    int *q = &b;
    EXPECT_TRUE(stdext::pointee_not_equal_to<int>()(p, q));
    EXPECT_FALSE(stdext::pointee_not_equal_to<int>()(p, p));
}

const int g_data[] = { 0, 1, 2, 3};
const int* g_pdata[] = { &g_data[0], &g_data[1], &g_data[2], &g_data[3] };
const int* g_reverse_pdata[] = { &g_data[3], &g_data[2], &g_data[1], &g_data[0] };

TEST(ComparePointee, BinarySearch)
{
    EXPECT_TRUE(
        std::binary_search(
            g_pdata, g_pdata + ARRAY_SIZE(g_pdata),
            &g_data[2],
            stdext::pointee_less<int>()
        )
    );

    int not_exist = 5;
    EXPECT_FALSE(
        std::binary_search(
            g_pdata, g_pdata + ARRAY_SIZE(g_pdata),
            &not_exist,
            stdext::pointee_less<int>()
        )
    );

    EXPECT_TRUE(
        std::binary_search(
            g_reverse_pdata, g_reverse_pdata + ARRAY_SIZE(g_reverse_pdata),
            &g_data[2],
            stdext::pointee_greater<int>()
        )
    );

    EXPECT_FALSE(
        std::binary_search(
            g_reverse_pdata, g_reverse_pdata + ARRAY_SIZE(g_reverse_pdata),
            &not_exist,
            stdext::pointee_greater<int>()
        )
    );
}

