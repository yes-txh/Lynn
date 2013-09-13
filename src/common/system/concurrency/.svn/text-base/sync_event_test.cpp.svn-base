// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2010-06-18

#include <iostream>
#include "common/base/stdint.h"
#include "common/system/concurrency/sync_event.hpp"
#include "common/system/concurrency/thread.hpp"
#include "common/system/time/timestamp.hpp"
#include "gtest/gtest.h"


void SetThread(SyncEvent* sync_event, volatile bool* done)
{
    for (int i = 0; i < 500000; ++i)
    {
        sync_event->Set();
    }
    *done = true;
    sync_event->Set();
}

TEST(SyncEvent, Test)
{
    volatile bool done = false;
    SyncEvent sync_event;
    Thread thread(Bind(SetThread, &sync_event, &done));
    thread.Start();
    while (!done)
    {
        uint64_t t0 = GetTimeStampInMs();
        sync_event.Wait();
        uint64_t t1 = GetTimeStampInMs();
        if (t1 - t0 > 100)
        {
            std::cout << "Wait Time: " << t1 - t0 << " ms" << std::endl;
        }
    }
}

const int kLoopCount = 100000;

void ProduceThread(SyncEvent* produce_event, SyncEvent* consume_event, int* n)
{
    for (int i = 0; i < kLoopCount; ++i)
    {
        produce_event->Set();
        consume_event->Wait();
    }
}

static void TestPerformance(int spin_count)
{
    SyncEvent produce_event(false, false, spin_count);
    SyncEvent consume_event(false, false, spin_count);
    int n = 0;
    Thread produce_thread(NewClosure(ProduceThread, &produce_event, &consume_event, &n));
    produce_thread.Start();
    for (int i = 0; i < kLoopCount; ++i)
    {
        produce_event.Wait();
        consume_event.Set();
    }
    produce_thread.Join();
}

TEST(SyncEvent, Performance)
{
    TestPerformance(0);
}

TEST(SyncEvent, SpinPerformance)
{
    TestPerformance(4000);
}

