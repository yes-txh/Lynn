// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2010-06-18

#ifndef COMMON_SYSTEM_CONCURRENCY_SPINLOCK_HPP
#define COMMON_SYSTEM_CONCURRENCY_SPINLOCK_HPP

#include <errno.h>
#include <stdlib.h>

#ifdef _WIN32

#include "common/system/concurrency/mutex.hpp"
typedef Mutex Spinlock;
#else

#include <pthread.h>
#include "common/system/concurrency/scoped_locker.hpp"
#include "common/system/concurrency/system_error.hpp"

/// spinlock is faster than mutex at some condition, but
class Spinlock
{
public:
    typedef ScopedLocker<Spinlock> Locker;
public:
    Spinlock()
    {
        CHECK_PTHREAD_ERROR(pthread_spin_init(&m_lock, 0));
    }

    ~Spinlock()
    {
        CHECK_PTHREAD_ERROR(pthread_spin_destroy(&m_lock));
    }

    void Lock()
    {
        CHECK_PTHREAD_ERROR(pthread_spin_lock(&m_lock));
    }

    bool TryLock()
    {
        return CHECK_PTHREAD_TRYLOCK_ERROR(pthread_spin_trylock(&m_lock));
    }

    void Unlock()
    {
        CHECK_PTHREAD_ERROR(pthread_spin_unlock(&m_lock));
    }
private:
    Spinlock(const Spinlock&);
    Spinlock& operator=(const Spinlock&);
private:
    pthread_spinlock_t m_lock;
};

#endif // _WIN32

// compatible reason
typedef Spinlock::Locker SpinlockLocker;

#endif // COMMON_SYSTEM_CONCURRENCY_SPINLOCK_HPP

