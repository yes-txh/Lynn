// Copyright (c) 2011, Tencent Inc. All rights reserved.
// ivanhuang @ 20101106

#include "common/system/concurrency/static_shm_queue.h"
#include "gtest/gtest.h"

// ���û���
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

// ���ý��ּ�
class ShmQueue : public testing::Test
{
protected:
    virtual void SetUp()
    {
        shm_key_    = 662;
        sem_key_    = 8864;
        block_size_ = 10<<10;
        block_num_  = 1<<10;

        // �����ڴ沢��ʼ��
        shm_queue_ = new StaticShmQueue;
        ASSERT_NE(-1, shm_queue_->Init(shm_key_, sem_key_, block_size_, block_num_));

        // ����ɾ�����
        // ������Ҫ������ڶ��β��Ի����֮ǰ������
        shm_queue_->SetRemoveFlag(true);
    }
    virtual void TearDown()
    {
        // ����ʼ�����ͷ��ڴ�
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

// ����getter����
TEST_F(ShmQueue, GetterFunction)
{
    // ���Թ����ڴ��key��id
    EXPECT_EQ(shm_queue_->GetShmKey(), shm_key_);
    EXPECT_GE(shm_queue_->GetShmId(), 0);

    // �����ź�����key��id
    EXPECT_EQ(shm_queue_->GetSemKey(), sem_key_);
    EXPECT_GE(shm_queue_->GetSemId(), 0);

    // ���Զ��нڵ�����ʹ�С
    EXPECT_EQ(shm_queue_->GetBlockSize(), block_size_);
    EXPECT_EQ(shm_queue_->Size(), 0);
}

// ���Ի�������
TEST_F(ShmQueue, BaseOperation)
{
    // ���Զ��пռ�
    EXPECT_TRUE(shm_queue_->HasSpace());

    char   **data_ptr  = new char *[block_num_];
    char   *out_data   = new char[block_size_];
    size_t buffer_size = block_size_;
    size_t out_size    = block_size_;

    // pop����������ʧ��
    ASSERT_EQ(shm_queue_->Pop(out_data, block_size_, &out_size), kErrShmEmpty);

    // push����
    for (int index = 0; index < block_num_; ++index)
    {
        data_ptr[index] = new char[block_size_];
        snprintf(data_ptr[index], block_size_, "ivan %d", index);

        // push�����������ɹ�
        ASSERT_TRUE(shm_queue_->Push(data_ptr[index], buffer_size));
    }

    // push����������ʧ��
    ASSERT_FALSE(shm_queue_->Push(data_ptr[0], buffer_size));

    // ���pop���ݣ����Ƿ�OK
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

// �����Ժ���
int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new MyTestEnvironment);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
