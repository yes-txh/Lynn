#include "common/base/stdext/algorithm.hpp"
#include <gtest/gtest.h>
#include "common/base/array_size.h"

TEST(Algorithm, binary_find)
{
    static const int numbers[] = { 1, 3, 4, 6, 8, 10 };
    const int* end = numbers + ARRAY_SIZE(numbers);
    EXPECT_EQ(end, stdext::binary_find(numbers, end, 0));
    EXPECT_EQ(end, stdext::binary_find(numbers, end, 5));
    EXPECT_EQ(end, stdext::binary_find(numbers, end, 11));
    EXPECT_EQ(numbers, stdext::binary_find(numbers, end, 1));
    EXPECT_EQ(numbers + 2, stdext::binary_find(numbers, end, 4));
    EXPECT_EQ(end - 1, stdext::binary_find(numbers, end, 10));
}

TEST(Algorithm, binary_find_pred)
{
    static const int numbers[] = { 1, 3, 4, 6, 8, 10 };
    const int* end = numbers + ARRAY_SIZE(numbers);
    EXPECT_EQ(end, stdext::binary_find(numbers, end, 0, std::less<int>()));
    EXPECT_EQ(end, stdext::binary_find(numbers, end, 5, std::less<int>()));
    EXPECT_EQ(end, stdext::binary_find(numbers, end, 11, std::less<int>()));
    EXPECT_EQ(numbers, stdext::binary_find(numbers, end, 1, std::less<int>()));
    EXPECT_EQ(numbers + 2, stdext::binary_find(numbers, end, 4, std::less<int>()));
    EXPECT_EQ(end - 1, stdext::binary_find(numbers, end, 10, std::less<int>()));
}

static bool is_odd(int n)
{
    return (n & 1) != 0;
}

TEST(Algorithm, copy_if)
{
    static const int src[] = { 1, 3, 4, 6, 9, 10 };
    int dest[ARRAY_SIZE(src)];
    ASSERT_EQ(dest + 3, stdext::copy_if(src, src + ARRAY_SIZE(src), dest, is_odd));
    ASSERT_EQ(1, dest[0]);
    ASSERT_EQ(3, dest[1]);
    ASSERT_EQ(9, dest[2]);
}

