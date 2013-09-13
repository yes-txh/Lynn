// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#undef NDEBUG
#include "common/base/global_initialize.hpp"
#include "common/system/concurrency/mutex.hpp"
#include "gtest/gtest.h"

GLOBAL_INITIALIZE(MutexTest)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";
}

TEST(Mutex, Lock)
{
    Mutex mutex;
    ASSERT_FALSE(mutex.IsLocked());
    mutex.Lock();
    ASSERT_TRUE(mutex.IsLocked());
    mutex.Unlock();
    ASSERT_FALSE(mutex.IsLocked());
}

TEST(Mutex, Locker)
{
    Mutex mutex;
    {
        ASSERT_FALSE(mutex.IsLocked());
        MutexLocker locker(mutex);
        ASSERT_TRUE(mutex.IsLocked());
    }
    ASSERT_FALSE(mutex.IsLocked());
}

template <typename Type>
void DestroyWithLockHolding()
{
    Type mutex;
    mutex.Lock();
}

TEST(Mutex, DestroyCheck)
{
    EXPECT_DEATH(DestroyWithLockHolding<AdaptiveMutex>(), "");
    EXPECT_DEATH(DestroyWithLockHolding<Mutex>(), "");
    EXPECT_DEATH(DestroyWithLockHolding<RecursiveMutex>(), "");
}

template <typename Type>
void RecursiveLocking()
{
    Type mutex;
    mutex.Lock();
    mutex.Lock();
    mutex.Unlock();
    mutex.Unlock();
}


TEST(Mutex, SelfDeadLock)
{
    EXPECT_DEATH(RecursiveLocking<Mutex>(), "");
    EXPECT_DEATH(RecursiveLocking<AdaptiveMutex>(), "");
    RecursiveLocking<RecursiveMutex>();
}

TEST(Mutex, LockerWithException)
{
    Mutex mutex;
    try
    {
        ASSERT_FALSE(mutex.IsLocked());
        MutexLocker locker(mutex);
        ASSERT_TRUE(mutex.IsLocked()) << "after locked constructed";
        throw 0;
    }
    catch (...)
    {
        // ignore
    }
    ASSERT_FALSE(mutex.IsLocked()) << "after exception thrown";
}
