// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/20/11

#include "common/system/concurrency/thread_group.hpp"
#include "common/system/concurrency/mutex.hpp"
#include "gtest/gtest.h"

class ThreadGroupTest : public testing::Test
{
public:
    enum { kCount = 100000 };
public:
    ThreadGroupTest() : n(0)
    {
    }
    void TestThread()
    {
        for (;;)
        {
            Mutex::Locker locker(mutex);
            if (++n >= kCount)
                return;
        }
    }
protected:
    int n;
    SimpleMutex mutex;
};

TEST_F(ThreadGroupTest, Test)
{
    ThreadGroup thread_group(Bind(&ThreadGroupTest::TestThread, this), 4);
    thread_group.Start();
    thread_group.Join();
    EXPECT_GE(n, kCount);
}

