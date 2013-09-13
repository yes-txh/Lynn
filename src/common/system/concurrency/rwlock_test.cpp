// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/system/concurrency/rwlock.hpp"
#include <gtest/gtest.h>

TEST(RWLock, Lock)
{
    RWLock lock;

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

TEST(RWLock, Locker)
{
    RWLock lock;

    ASSERT_FALSE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());

    {
        RWLock::ReaderLocker locker(lock);
        ASSERT_TRUE(lock.IsLocked());
        ASSERT_TRUE(lock.IsReaderLocked());
        ASSERT_FALSE(lock.IsWriterLocked());
    }

    ASSERT_FALSE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());

    {
        RWLock::WriterLocker locker(lock);
        ASSERT_TRUE(lock.IsLocked());
        ASSERT_FALSE(lock.IsReaderLocked());
        ASSERT_TRUE(lock.IsWriterLocked());
    }

    ASSERT_FALSE(lock.IsLocked());
    ASSERT_FALSE(lock.IsReaderLocked());
    ASSERT_FALSE(lock.IsWriterLocked());
}

TEST(RWLock, ReaderLockerWithException)
{
    RWLock lock;
    try
    {
        RWLock::ReaderLocker locker(lock);
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

TEST(RWLock, WriterLockerWithException)
{
    RWLock lock;
    try
    {
        RWLock::WriterLocker locker(lock);
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

TEST(RWLock, TryLock)
{
    RWLock lock;

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
}
