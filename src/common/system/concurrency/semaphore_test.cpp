// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/system/concurrency/semaphore.hpp"
#include <gtest/gtest.h>

TEST(Semaphore, Init)
{
    Semaphore sem(2);
    ASSERT_EQ(sem.GetValue(), 2U);
    sem.Wait();
    ASSERT_EQ(sem.GetValue(), 1U);
    sem.Wait();
    ASSERT_EQ(sem.GetValue(), 0U);
    ASSERT_FALSE(sem.TryWait());
    ASSERT_EQ(sem.GetValue(), 0U);
}

TEST(Semaphore, Wait)
{
    Semaphore sem(1);
    ASSERT_EQ(sem.GetValue(), 1U);
    sem.Wait();
    ASSERT_EQ(sem.GetValue(), 0U);
    ASSERT_FALSE(sem.TryWait());
    ASSERT_EQ(sem.GetValue(), 0U);
}

TEST(Semaphore, Release)
{
    Semaphore sem(1);
    ASSERT_EQ(sem.GetValue(), 1U);
    sem.Wait();
    ASSERT_EQ(sem.GetValue(), 0U);
    ASSERT_FALSE(sem.TryWait());
    ASSERT_EQ(sem.GetValue(), 0U);
    sem.Release();
    ASSERT_EQ(sem.GetValue(), 1U);
    ASSERT_TRUE(sem.TryWait());
}

TEST(Semaphore, TimedWait)
{
    Semaphore sem(1);
    sem.Wait();
    ASSERT_FALSE(sem.TryWait());
    time_t t0 = time(NULL);
    ASSERT_FALSE(sem.TimedWait(1100));
    time_t t1 = time(NULL);
    ASSERT_GT(t1, t0);
}

#ifndef _WIN32

// 测试构造和初始化函数 ivanhuang
TEST(SemaphoreLock, TestCreate)
{
    // 设置特定key值，构造信号量
    const int kKey = 6628864;
    SemaphoreLock sem_lock;
    ASSERT_NE(-1, sem_lock.InitSem(kKey));

    // 获取key值，期望与设置时相等
    int key_id = sem_lock.GetSemKey();
    EXPECT_EQ(kKey, key_id);

    // 再次以特定key值构造信号量
    SemaphoreLock sem_lock_1;
    ASSERT_NE(-1, sem_lock_1.InitSem(kKey));

    // 获取两个对象的sem_id值，期望相等
    EXPECT_EQ(sem_lock.GetSemId(), sem_lock_1.GetSemId());
}

// 测试PV操作 ivanhuang
TEST(SemaphoreLock, TestPV)
{
    // 设置特定key值，构造信号量
    const int kKey = 6628864;
    SemaphoreLock sem_lock;
    ASSERT_NE(-1, sem_lock.InitSem(kKey));

    // 测试信号量锁的当前状态
    EXPECT_FALSE(sem_lock.IsLock());

    // 测试P操作
    EXPECT_TRUE(sem_lock.Acquire());

    // 测试信号量锁的当前状态
    EXPECT_TRUE(sem_lock.IsLock());

    // 测试V操作
    EXPECT_TRUE(sem_lock.Release());

    // 测试信号量锁的当前状态
    EXPECT_FALSE(sem_lock.IsLock());
}

#endif // _WIN32
