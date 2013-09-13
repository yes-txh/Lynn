// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASELIB_SVRPUBLIB_WRAPPER_RWLOCK_H_
#define COMMON_BASELIB_SVRPUBLIB_WRAPPER_RWLOCK_H_

// copy from common

/// @file
/// @author phongche <phongchen@tencent.com>
/// @date Nov 28, 2010

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <string>

#include "common/baselib/svrpublib/thread_mutex.h"


_START_XFS_BASE_NAMESPACE_

#if defined __unix__

#include <pthread.h>

// Reader/Writer lock
class RWLocker
{
public:
    RWLocker()
    {
        pthread_rwlock_init(&m_lock, NULL);
    }
    ~RWLocker()
    {
        pthread_rwlock_destroy(&m_lock);
    }

    void ReaderLock()
    {
        pthread_rwlock_rdlock(&m_lock);
    }

    bool TryReaderLock()
    {
        return pthread_rwlock_tryrdlock(&m_lock) == 0;
    }

    void WriterLock()
    {
        pthread_rwlock_wrlock(&m_lock);
    }

    bool TryWriterLock()
    {
        return pthread_rwlock_trywrlock(&m_lock) == 0;
    }

    void Unlock()
    {
        pthread_rwlock_unlock(&m_lock);
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
    RWLocker(const RWLocker& src);
    RWLocker& operator=(const RWLocker& rhs);
private:
    pthread_rwlock_t m_lock;
};

#elif defined _WIN32


// fallback to mutex
class RWLocker
{
public:
    RWLocker() : m_lock_count(0)
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
        m_mutex.UnLock();
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
    xfs::base::ThreadMutex m_mutex;

    // > 0: reader locked
    // < 0: writer locker
    // = 0: unlocked
    int m_lock_count;
};


#endif


_END_XFS_BASE_NAMESPACE_


#endif // COMMON_BASELIB_SVRPUBLIB_WRAPPER_RWLOCK_H_
