// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2010-06-18

#include "common/system/concurrency/mutex.hpp"
#include "common/system/concurrency/spinlock.hpp"
#include "common/system/concurrency/thread_group.hpp"
#include "gtest/gtest.h"

const int test_count = 10000000;

TEST(MutexTest, Mutex)
{
    SimpleMutex lock;
    for (int i = 0; i < test_count; ++i)
    {
        Mutex::Locker locker(lock);
    }
}

void TestThread(int* p, SimpleMutex* mutex)
{
    for (;;)
    {
        Mutex::Locker locker(mutex);
        if (++(*p) >= test_count)
            return;
    }
}

TEST(MutexTest, ThreadMutex)
{
    int n = 0;
    SimpleMutex lock;
    ThreadGroup thread_group(Bind(TestThread, &n, &lock), 4);
    thread_group.Start();
    thread_group.Join();
}

TEST(SpinlockTest, Spinlock)
{
    Spinlock lock;
    for (int i = 0; i < test_count; ++i)
    {
        Spinlock::Locker locker(lock);
    }
}
