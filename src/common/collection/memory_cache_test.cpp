// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 06/22/11
// Description:

#include "common/collection/memory_cache.hpp"
#include "gtest/gtest.h"

TEST(MemoryCache, Capacity)
{
    MemoryCache<int, int> cache(3);
    EXPECT_EQ(3U, cache.Capacity());
}

TEST(MemoryCache, Size)
{
    MemoryCache<int, int> cache(3);
    EXPECT_EQ(0U, cache.Size());
    EXPECT_TRUE(cache.IsEmpty());

    cache.Put(1, 0);
    EXPECT_EQ(1U, cache.Size());
    EXPECT_FALSE(cache.IsEmpty());
    EXPECT_FALSE(cache.IsFull());

    cache.Put(2, 0);
    EXPECT_EQ(2U, cache.Size());
    EXPECT_FALSE(cache.IsEmpty());
    EXPECT_FALSE(cache.IsFull());

    cache.Put(3, 0);
    EXPECT_EQ(3U, cache.Size());
    EXPECT_FALSE(cache.IsEmpty());
    EXPECT_TRUE(cache.IsFull());

    cache.Put(4, 0);
    EXPECT_EQ(3U, cache.Size());
    EXPECT_FALSE(cache.IsEmpty());
    EXPECT_TRUE(cache.IsFull());
}

TEST(MemoryCache, Insert)
{
    MemoryCache<int, int> cache(3);
    EXPECT_TRUE(cache.Insert(1, 0));
    EXPECT_TRUE(cache.Insert(2, 0));
    EXPECT_FALSE(cache.Insert(1, 0));
    EXPECT_FALSE(cache.Insert(2, 0));
}

TEST(MemoryCache, Replace)
{
    MemoryCache<int, int> cache(3);
    EXPECT_TRUE(cache.Insert(1, 0));
    EXPECT_TRUE(cache.Insert(2, 0));
    EXPECT_TRUE(cache.Replace(1, 0));
    EXPECT_TRUE(cache.Replace(2, 0));
    EXPECT_FALSE(cache.Replace(3, 0));
}

TEST(MemoryCache, Put)
{
    MemoryCache<int, int> cache(3);
    EXPECT_TRUE(cache.Put(1, 1));
    EXPECT_TRUE(cache.Put(1, 1));
    EXPECT_TRUE(cache.Put(2, 1));
    EXPECT_TRUE(cache.Put(3, 1));
    EXPECT_TRUE(cache.Put(4, 1));
    EXPECT_TRUE(cache.Put(5, 1));
}

TEST(MemoryCache, Get)
{
    MemoryCache<int, int> cache(3);
    EXPECT_TRUE(cache.Put(1, 1));

    int value = 0;
    EXPECT_TRUE(cache.Get(1, &value));
    EXPECT_EQ(1, value);

    EXPECT_FALSE(cache.Get(0, &value));
}

TEST(MemoryCache, GetOrDefault)
{
    MemoryCache<int, int> cache(3);
    EXPECT_TRUE(cache.Put(1, 1));
    EXPECT_EQ(1, cache.GetOrDefault(1));
    EXPECT_EQ(0, cache.GetOrDefault(2));
}

TEST(MemoryCache, Remove)
{
    MemoryCache<int, int> cache(3);
    EXPECT_FALSE(cache.Remove(0));
    cache.Put(0, 1);
    EXPECT_TRUE(cache.Remove(0));
    EXPECT_FALSE(cache.Remove(0));
}

TEST(MemoryCache, Discard)
{
    MemoryCache<int, int> cache(3);
    cache.Put(1, 0);
    cache.Put(2, 0);
    cache.Put(3, 0);
    cache.Put(4, 0);
    EXPECT_TRUE(cache.Contains(4));
    EXPECT_TRUE(cache.Contains(3));
    EXPECT_TRUE(cache.Contains(2));
    EXPECT_FALSE(cache.Contains(1));
}

TEST(MemoryCache, Clear)
{
    MemoryCache<int, int> cache(3);
    cache.Put(1, 0);
    EXPECT_FALSE(cache.IsEmpty());
    cache.Clear();
    EXPECT_TRUE(cache.IsEmpty());
}

TEST(MemoryCache, Iteration)
{
    MemoryCache<int, int> cache(3);
    cache.Put(1, 0);
    cache.Put(2, 0);
    cache.Put(3, 0);
    cache.Put(4, 0);

    int key = 0, value = 0;
    ASSERT_TRUE(cache.First(&key, &value));
    printf("key = %d, value = %d\n", key, value);

    ASSERT_TRUE(cache.Next(&key, &value));
    printf("key = %d, value = %d\n", key, value);

    ASSERT_TRUE(cache.Next(&key, &value));
    printf("key = %d, value = %d\n", key, value);

    ASSERT_FALSE(cache.Next(&key, &value));
}

TEST(MemoryCache, InsertPerformance)
{
    MemoryCache<int, int> cache(10000);
    for (int i = 0; i < 1000000; ++i)
        cache.Put(i, i);
}
