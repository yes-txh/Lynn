// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/system/concurrency/lite_rwlock.hpp"
#include "common/system/concurrency/rwlock.hpp"
#include "gtest/gtest.h"

TEST(LiteRWLock, Lock)
{
    LiteRWLock lock;

    ASSERT_FALSE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());

    lock.ReaderLock();
    ASSERT_TRUE(lock.IsLocked());
    ASSERT_TRUE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());


    lock.ReaderUnlock();
    ASSERT_FALSE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());

    lock.WriterLock();
    ASSERT_TRUE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_TRUE(lock.IsWriterLocked());

    lock.WriterUnlock();
    ASSERT_FALSE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());
}

TEST(LiteRWLock, Locker)
{
    LiteRWLock lock;

    ASSERT_FALSE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());

    {
        LiteRWLock::ReaderLocker locker(lock);
        ASSERT_TRUE(lock.IsLocked());
        ASSERT_TRUE(lock.IsReaderLocked());
        ASSERT_FALSE(lock.IsWriterLocked());
    }

    ASSERT_FALSE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());

    {
        LiteRWLock::WriterLocker locker(lock);
        ASSERT_TRUE(lock.IsLocked());
        ASSERT_FALSE(lock.IsReaderLocked());
        ASSERT_TRUE(lock.IsWriterLocked());
    }

    ASSERT_FALSE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());
}

TEST(LiteRWLock, ReaderLockerWithException)
{
    LiteRWLock lock;
    try
    {
        LiteRWLock::ReaderLocker locker(lock);
        ASSERT_TRUE(lock.IsLocked());
        ASSERT_TRUE(lock.IsReaderLocked());
        ASSERT_FALSE(lock.IsWriterLocked());
        throw 0;
    }
    catch (...)
    {
        // ignore
    }
    ASSERT_FALSE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());
}

TEST(LiteRWLock, WriterLockerWithException)
{
    LiteRWLock lock;
    try
    {
        LiteRWLock::WriterLocker locker(lock);
        ASSERT_TRUE(lock.IsLocked());
        ASSERT_FALSE(lock.IsReaderLocked());
        ASSERT_TRUE(lock.IsWriterLocked());
        throw 0;
    }
    catch (...)
    {
        // ignore
    }
    ASSERT_FALSE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());
}

TEST(LiteRWLock, TryLock)
{
    LiteRWLock lock;

    ASSERT_FALSE(lock.IsLocked());
    ASSERT_TRUE(lock.TryReaderLock());
    ASSERT_TRUE(lock.IsLocked());
    ASSERT_TRUE(lock.TryReaderLock());
    ASSERT_TRUE(lock.TryReaderLock());
    ASSERT_FALSE(lock.TryWriterLock());
    lock.ReaderUnlock();
    ASSERT_TRUE(lock.IsLocked());
    lock.ReaderUnlock();
    ASSERT_TRUE(lock.IsLocked());
    lock.ReaderUnlock();
    ASSERT_FALSE(lock.IsLocked());
    ASSERT_TRUE(lock.TryWriterLock());
    ASSERT_FALSE(lock.TryWriterLock());
    ASSERT_FALSE(lock.TryReaderLock());
    lock.WriterUnlock();
    ASSERT_FALSE(lock.IsLocked());

    {
        LiteRWLock::TryReaderLocker read_locker(lock);
    }

    {
        LiteRWLock::TryWriterLocker writer_locker(lock);
    }
}

const int kTestLoopCount = 10000000;

template <typename LockType>
class RWLockTest: public testing::Test
{
};

TYPED_TEST_CASE_P(RWLockTest);

TYPED_TEST_P(RWLockTest, ReaderPerformance)
{
    TypeParam lock;
    for (int i = 0; i < kTestLoopCount; ++i)
    {
        typename TypeParam::ReaderLocker locker(lock);
    }
}

TYPED_TEST_P(RWLockTest, WriterPerformance)
{
    TypeParam lock;
    for (int i = 0; i < kTestLoopCount; ++i)
    {
        typename TypeParam::WriterLocker locker(lock);
    }
}

REGISTER_TYPED_TEST_CASE_P(RWLockTest, ReaderPerformance, WriterPerformance);

typedef testing::Types<RWLock, LiteRWLock> TestTypes;
INSTANTIATE_TYPED_TEST_CASE_P(Performance, RWLockTest, TestTypes);

