// Copyright (c) 2011, Tencent Inc. All rights reserved.
// ivanhuang @ 20101106

#include "common/system/concurrency/static_shm_queue.h"
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
        block_size_ = 10<<10;
        block_num_  = 1<<10;

        // 分配内存并初始化
        shm_queue_ = new StaticShmQueue;
        ASSERT_NE(-1, shm_queue_->Init(shm_key_, sem_key_, block_size_, block_num_));

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
    int    block_size_;
    int    block_num_;

    StaticShmQueue *shm_queue_;
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

    // 测试队列节点个数和大小
    EXPECT_EQ(shm_queue_->GetBlockSize(), block_size_);
    EXPECT_EQ(shm_queue_->Size(), 0);
}

// 测试基本操作
TEST_F(ShmQueue, BaseOperation)
{
    // 测试队列空间
    EXPECT_TRUE(shm_queue_->HasSpace());

    char   **data_ptr  = new char *[block_num_];
    char   *out_data   = new char[block_size_];
    size_t buffer_size = block_size_;
    size_t out_size    = block_size_;

    // pop操作，期望失败
    ASSERT_EQ(shm_queue_->Pop(out_data, block_size_, &out_size), kErrShmEmpty);

    // push数据
    for (int index = 0; index < block_num_; ++index)
    {
        data_ptr[index] = new char[block_size_];
        snprintf(data_ptr[index], block_size_, "ivan %d", index);

        // push操作，期望成功
        ASSERT_TRUE(shm_queue_->Push(data_ptr[index], buffer_size));
    }

    // push操作，期望失败
    ASSERT_FALSE(shm_queue_->Push(data_ptr[0], buffer_size));

    // 随机pop数据，看是否OK
    srand(time(NULL));
    size_t select   = 0;
    size_t get_size = 0;

    for (int i = 0; i < 20; ++i)
    {
        select = rand() % block_num_;

        ASSERT_EQ(shm_queue_->GetValue(out_data, block_size_, select, &get_size), 0);
        ASSERT_STREQ(out_data, data_ptr[select]);
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
