// Copyright (c) 2011, Tencent Inc. All rights reserved.
// ivanhuang @ 20101101

#include "common/system/concurrency/circle_queue.h"
#include "gtest/gtest.h"

struct TestForQueue {
    int   i;
    float f;
};

TEST(CircleQueue, BaseOperation) {
    const int kQueueSize = 1 << 10;
    CircleQueue<TestForQueue> queue;

    ASSERT_TRUE(queue.Init(kQueueSize));
    ASSERT_EQ(kQueueSize, queue.Capacity());
    ASSERT_EQ(0, queue.Length());
    ASSERT_TRUE(queue.Empty());
    ASSERT_FALSE(queue.Full());

    TestForQueue test = {1, 2.0};
    ASSERT_TRUE(queue.Push(&test));
    ASSERT_EQ(kQueueSize, queue.Capacity());
    ASSERT_EQ(1, queue.Length());
    ASSERT_FALSE(queue.Empty());
    ASSERT_FALSE(queue.Full());

    char data[1024];
    int data_len;

    ASSERT_TRUE(queue.Pop(data, data_len));
    ASSERT_EQ(kQueueSize, queue.Capacity());
    ASSERT_EQ(0, queue.Length());
    ASSERT_TRUE(queue.Empty());
    ASSERT_FALSE(queue.Full());
    ASSERT_NE(static_cast<int>(sizeof(test)), data_len);

    ASSERT_TRUE(queue.Push(&test));
    ASSERT_TRUE(queue.Pop(reinterpret_cast<TestForQueue *>(data)));
    TestForQueue *test_data = reinterpret_cast<TestForQueue *>(data);

    EXPECT_EQ(1, test_data->i);

    double diff = 2.0 - test_data->f;
    diff = diff < 0 ? -diff : diff;
    EXPECT_LT(diff, 0.001);

    for (int i = 0; i < kQueueSize; ++i) {
        ASSERT_TRUE(queue.Push(&test));
    }

    ASSERT_TRUE(queue.Full());
}

