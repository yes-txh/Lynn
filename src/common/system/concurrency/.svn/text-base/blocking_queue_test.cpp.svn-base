// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/system/concurrency/blocking_queue.hpp"
#include "common/system/concurrency/thread.hpp"
#include "common/system/time/timestamp.hpp"
#include "gtest/gtest.h"

TEST(BlockingQueue, Init)
{
    BlockingQueue<int> queue;
    queue.PushBack(1);
    int n;
    ASSERT_TRUE(queue.TimedPopFront(&n, 0));
    ASSERT_EQ(1, n);
}

TEST(BlockingQueue, Full)
{
    // Initialize a queue with capacity 10.
    static const int kCapacity = 10;
    int n;
    BlockingQueue<int> queue(kCapacity);
    std::deque<int> values;

    // The queue is empty now.
    ASSERT_TRUE(queue.IsEmpty());
    ASSERT_FALSE(queue.TimedPopBack(&n, 0));
    ASSERT_FALSE(queue.TimedPopAll(&values, 0));

    // Push some elmenets.
    for (int i = 0; i < kCapacity; ++i) {
        ASSERT_FALSE(queue.IsFull());
        queue.PushBack(i);
    }

    // The queue is full now.
    ASSERT_TRUE(queue.IsFull());
    ASSERT_FALSE(queue.TimedPushBack(0, 0));

    // Pop from back.
    ASSERT_TRUE(queue.TimedPopBack(&n, 0));
    ASSERT_EQ(kCapacity - 1, n);

    // The queue is not full now.
    ASSERT_FALSE(queue.IsFull());
    queue.PushBack(kCapacity);
    ASSERT_TRUE(queue.TimedPopBack(&n, 0));
    ASSERT_EQ(kCapacity, n);

    // The queue is full now.
    queue.PushBack(kCapacity);
    ASSERT_TRUE(queue.IsFull());
    ASSERT_FALSE(queue.TimedPushBack(0, 0));

    ASSERT_TRUE(queue.TimedPopAll(&values, 0));
    ASSERT_EQ(static_cast<size_t>(kCapacity), values.size());

    // The queue is empty now.
    ASSERT_FALSE(queue.TimedPopBack(&n, 0));
}

void QueueThread(BlockingQueue<int64_t>* queue)
{
    int t = 0;
    for (int i = 0; i < 1000000; ++i)
    {
        queue->PushBack(++t);
    }
    queue->PushBack(-1);
}

TEST(BlockingQueue, Performance)
{
    BlockingQueue<int64_t> queue;
    Thread thread(Bind(QueueThread, &queue));
    thread.Start();

    for (;;)
    {
        int64_t t;
        queue.PopFront(&t);
        if (t < 0)
            break;
    }

    thread.Join();
}

