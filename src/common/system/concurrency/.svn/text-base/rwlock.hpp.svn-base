// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_SYSTEM_CONCURRENCY_RWLOCK_HPP
#define COMMON_SYSTEM_CONCURRENCY_RWLOCK_HPP

/// @file
/// @author phongche <phongchen@tencent.com>
/// @date Nov 28, 2010

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <string>

#include "common/system/concurrency/scoped_locker.hpp"
#include "common/system/concurrency/system_error.hpp"

#if defined __unix__

#include <pthread.h>

// Reader/Writer lock
class RWLock
{
public:
    typedef ScopedReaderLocker<RWLock> ReaderLocker;
    typedef ScopedWriterLocker<RWLock> WriterLocker;

public:
    RWLock()
    {
        CHECK_PTHREAD_ERROR(pthread_rwlock_init(&m_lock, NULL));
    }
    ~RWLock()
    {
        CHECK_PTHREAD_ERROR(pthread_rwlock_destroy(&m_lock));
    }

    void ReaderLock()
    {
        CHECK_PTHREAD_ERROR(pthread_rwlock_rdlock(&m_lock));
    }

    bool TryReaderLock()
    {
        return CHECK_PTHREAD_TRYLOCK_ERROR(pthread_rwlock_tryrdlock(&m_lock));
    }

    void WriterLock()
    {
        CHECK_PTHREAD_ERROR(pthread_rwlock_wrlock(&m_lock));
    }

    bool TryWriterLock()
    {
        return CHECK_PTHREAD_TRYLOCK_ERROR(pthread_rwlock_trywrlock(&m_lock));
    }

    void Unlock()
    {
        CHECK_PTHREAD_ERROR(pthread_rwlock_unlock(&m_lock));
    }

    // names for scoped lockers
    void ReaderUnlock()
    {
        Unlock();
    }

    void WriterUnlock()
    {
        Unlock();
    }

public: // only for test and debug, should not be used in normal code logical
    bool IsReaderLocked() const
    {
        // accessing pthread private data: nonportable but no other way
        return m_lock.__data.__nr_readers != 0;
    }

    bool IsWriterLocked() const
    {
        return m_lock.__data.__writer != 0;
    }

    bool IsLocked() const
    {
        return IsReaderLocked() || IsWriterLocked();
    }

private: // forbid copying
    RWLock(const RWLock& src);
    RWLock& operator=(const RWLock& rhs);
private:
    pthread_rwlock_t m_lock;
};

#elif defined _WIN32

#include "common/system/concurrency/mutex.hpp"

// fallback to mutex
class RWLock
{
public:
    typedef ScopedReaderLocker<RWLock> ReaderLocker;
    typedef ScopedWriterLocker<RWLock> WriterLocker;
public:
    RWLock() : m_lock_count(0)
    {
    }

    void ReaderLock()
    {
        m_mutex.Lock();
        ++m_lock_count;
    }

    bool TryReaderLock()
    {
        bool ret = m_mutex.TryLock();
        if (ret)
            ++m_lock_count;
        return ret;
    }

    void WriterLock()
    {
        m_mutex.Lock();
        --m_lock_count;
    }

    bool TryWriterLock()
    {
        bool ret = m_mutex.TryLock();
        if (ret)
            ++m_lock_count;
        return ret;
    }

    void Unlock()
    {
        m_lock_count = 0;
        m_mutex.Unlock();
    }

    void ReaderUnlock()
    {
        Unlock();
    }

    void WriterUnlock()
    {
        Unlock();
    }

public: // only for test and debug, should not be used in normal code logical
    bool IsReaderLocked() const
    {
        return m_lock_count > 0;
    }

    bool IsWriterLocked() const
    {
        return m_lock_count < 0;
    }

    bool IsLocked() const
    {
        return m_lock_count != 0;
    }

private:
    // mutex is noncopyable, so copy ctor and assign can't be generated
    Mutex m_mutex;

    // > 0: reader locked
    // < 0: writer locker
    // = 0: unlocked
    int m_lock_count;
};


#endif

#endif // COMMON_SYSTEM_CONCURRENCY_RWLOCK_HPP
