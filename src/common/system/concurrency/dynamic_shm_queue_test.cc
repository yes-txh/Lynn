// Copyright (c) 2011, Tencent Inc. All rights reserved.
// ivanhuang @ 20101106

#include "common/system/concurrency/dynamic_shm_queue.h"
#include "gtest/gtest.h"

// 设置环境
class MyTestEnvironment : public testing::Environment
{
public:
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
};

#ifndef _WIN32

// 设置脚手架
class ShmQueue : public testing::Test
{
protected:
    virtual void SetUp()
    {
        shm_key_    = 662;
        sem_key_    = 8864;
        queue_size_ = 10<<10;

        // 分配内存并初始化
        shm_queue_ = new DynamicShmQueue;
        ASSERT_TRUE((bool)shm_queue_->Init(shm_key_, sem_key_, queue_size_));

        // 设置删除标记
        // 测试需要，否则第二次测试会加载之前的数据
        shm_queue_->SetRemoveFlag(true);
    }
    virtual void TearDown()
    {
        // 反初始化并释放内存
        delete shm_queue_;
        shm_queue_ = NULL;
    }

protected:
    int    shm_key_;
    int    sem_key_;
    int    queue_size_;

    DynamicShmQueue *shm_queue_;
};

// 测试getter函数
TEST_F(ShmQueue, GetterFunction)
{
    // 测试共享内存的key和id
    EXPECT_EQ(shm_queue_->GetShmKey(), shm_key_);
    EXPECT_GE(shm_queue_->GetShmId(), 0);

    // 测试信号量的key和id
    EXPECT_EQ(shm_queue_->GetSemKey(), sem_key_);
    EXPECT_GE(shm_queue_->GetSemId(), 0);

    // 测试队列长度和数据长度
    EXPECT_EQ(shm_queue_->GetQueueSize(), queue_size_);
    EXPECT_EQ(shm_queue_->GetQueueDataLen(), 0);
}

// 测试基本操作
TEST_F(ShmQueue, BaseOperation)
{
    const int kBlockNum  = 15;
    const int kBlockSize = 1<<10;

    char **data_ptr  = new char *[kBlockNum];
    char *out_data   = new char[kBlockSize];
    int  buffer_size = kBlockSize;
    int  out_size    = kBlockSize;

    // pop操作，期望失败
    ASSERT_FALSE(shm_queue_->Pop(out_data, kBlockSize, &out_size));

    // push数据
    for (int index = 0; index < 9; ++index)
    {
        data_ptr[index] = new char[kBlockSize];
        snprintf(data_ptr[index], kBlockSize, "ivan %d", index);

        // push操作，期望成功
        ASSERT_TRUE((bool)shm_queue_->Push(data_ptr[index], buffer_size));
    }

    for (int index = 9; index < kBlockNum; ++index)
    {
        data_ptr[index] = new char[kBlockSize];
        snprintf(data_ptr[index], kBlockSize, "ivan %d", index);

        // push操作，期望失败
        ASSERT_FALSE(shm_queue_->Push(data_ptr[index], buffer_size));
    }

    // 随机pop数据，看是否OK
    srand(time(NULL));
    int get_size = 0;

    for (int i = 0; i < 9; ++i)
    {
        ASSERT_TRUE((bool)shm_queue_->Pop(out_data, buffer_size, &get_size));
    }
}

#endif // _WIN32

// 主测试函数
int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new MyTestEnvironment);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
