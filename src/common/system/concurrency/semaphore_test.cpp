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

// ���Թ���ͳ�ʼ������ ivanhuang
TEST(SemaphoreLock, TestCreate)
{
    // �����ض�keyֵ�������ź���
    const int kKey = 6628864;
    SemaphoreLock sem_lock;
    ASSERT_NE(-1, sem_lock.InitSem(kKey));

    // ��ȡkeyֵ������������ʱ���
    int key_id = sem_lock.GetSemKey();
    EXPECT_EQ(kKey, key_id);

    // �ٴ����ض�keyֵ�����ź���
    SemaphoreLock sem_lock_1;
    ASSERT_NE(-1, sem_lock_1.InitSem(kKey));

    // ��ȡ���������sem_idֵ���������
    EXPECT_EQ(sem_lock.GetSemId(), sem_lock_1.GetSemId());
}

// ����PV���� ivanhuang
TEST(SemaphoreLock, TestPV)
{
    // �����ض�keyֵ�������ź���
    const int kKey = 6628864;
    SemaphoreLock sem_lock;
    ASSERT_NE(-1, sem_lock.InitSem(kKey));

    // �����ź������ĵ�ǰ״̬
    EXPECT_FALSE(sem_lock.IsLock());

    // ����P����
    EXPECT_TRUE(sem_lock.Acquire());

    // �����ź������ĵ�ǰ״̬
    EXPECT_TRUE(sem_lock.IsLock());

    // ����V����
    EXPECT_TRUE(sem_lock.Release());

    // �����ź������ĵ�ǰ״̬
    EXPECT_FALSE(sem_lock.IsLock());
}

#endif // _WIN32
