// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/13/11
// Description:

#include "common/system/concurrency/thread.hpp"
#include "gtest/gtest.h"

void ThreadCallback(int* p)
{
    ++*p;
}

TEST(Thread, Test)
{
    int n = 0;
    Thread thread(Bind(ThreadCallback, &n));
    thread.Start();
    thread.Join();
    EXPECT_EQ(1, n);
}

void DoNothing()
{
}

TEST(Thread, Restart)
{
    Thread thread(Bind(DoNothing));
    for (int i = 0; i < 1000; ++i)
    {
        thread.Start();
        int tid1 = thread.GetId();
        thread.Join();

        thread.Start();
        int tid2 = thread.GetId();
        EXPECT_NE(tid1, tid2);
        thread.Join();
    }
}

void IsRunningTestThread(volatile const bool* stop)
{
    while (!*stop)
        ThisThread::Sleep(1);
    ThisThread::Exit();
}

TEST(Thread, IsRunning)
{
    bool stop = false;
    Thread thread(Bind(IsRunningTestThread, &stop));
    thread.Start();
    for (int i = 0; i < 1000; ++i)
    {
        if (!thread.IsRunning())
            ThisThread::Sleep(1);
    }
    stop = true;
    thread.Join();
    EXPECT_FALSE(thread.IsRunning());
}
