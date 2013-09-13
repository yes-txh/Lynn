// Copyright (c) 2009, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/base/object_pool.hpp"

#include <string>
#include "common/system/concurrency/spinlock.hpp"
#include "gtest/gtest.h"

TEST(ObjectPool, ObjectPool)
{
    ObjectPool<int, NullClear> pool(1024);

    int *p = pool.Acquire();
    pool.Release(p);

    // 分配对象以占领地址，避免下次 new 恰好得到上一次的地址
    int *p1 = new int();
    *p1 = 0;

    int *q = pool.Acquire();
    pool.Release(p);
    ASSERT_TRUE(p == q);
    delete p1;
}

TEST(FixedObjectPool, FixedObjectPool)
{
    FixedObjectPool<std::string, 16, CallMember_clear> pool;
    std::string *p = pool.Acquire();
    pool.Release(p);
}

template <typename T>
T* NullPtr()
{
    return 0;
}

TEST(ObjectPool, Quota)
{
    ObjectPool<int> pool(1, 1, false);
    int* p1 = pool.Acquire();
    EXPECT_NE(NullPtr<int>(), p1);
    int* p2 = pool.Acquire();
    EXPECT_EQ(NullPtr<int>(), p2);
    pool.Release(p1);
}

TEST(FixedObjectPool, Quota)
{
    FixedObjectPool<int, 1> pool(1, false);
    EXPECT_EQ(1U, pool.Size());
    int* p1 = pool.Acquire();
    EXPECT_EQ(0U, pool.Size());
    EXPECT_NE(NullPtr<int>(), p1);
    int* p2 = pool.Acquire();
    EXPECT_EQ(NullPtr<int>(), p2);
    pool.Release(p1);
}

TEST(ObjectPool, QuotaAutoCreate)
{
    ObjectPool<int> pool(1, 1, true);
    EXPECT_EQ(1U, pool.Size());
    int* p1 = pool.Acquire();
    EXPECT_EQ(0U, pool.Size());
    EXPECT_NE(NullPtr<int>(), p1);
    int* p2 = pool.Acquire();
    EXPECT_NE(NullPtr<int>(), p2);
    pool.Release(p1);
    pool.Release(p2);
}

TEST(FixedObjectPool, QuotaAutoCreate)
{
    FixedObjectPool<int, 1> pool(1, true);
    int* p1 = pool.Acquire();
    EXPECT_NE(NullPtr<int>(), p1);
    int* p2 = pool.Acquire();
    EXPECT_NE(NullPtr<int>(), p2);
    pool.Release(p1);
    pool.Release(p2);
}

const int kLoopCount = 1000000;

class ObjectPoolPerformaceTest : public testing::Test
{
};

TEST_F(ObjectPoolPerformaceTest, SpinlockPool)
{
    FixedObjectPool<std::string, 1024, CallMember_clear, Spinlock> pool;
    for (int i = 0; i < kLoopCount; ++i)
    {
        std::string* p = pool.Acquire();
        p->resize(256);
        pool.Release(p);
    }
}

TEST_F(ObjectPoolPerformaceTest, MutexPool)
{
    FixedObjectPool<std::string, 1024, CallMember_clear, SimpleMutex> pool;
    for (int i = 0; i < kLoopCount; ++i)
    {
        std::string* p = pool.Acquire();
        p->resize(256);
        pool.Release(p);
    }
}

TEST_F(ObjectPoolPerformaceTest, NewDelete)
{
    for (int i = 0; i < kLoopCount; ++i)
    {
        std::string* p = new std::string();
        p->resize(256);
        delete p;
    }
}

