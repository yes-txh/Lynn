// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_SYSTEM_CONCURRENCY_LITE_RWLOCK_HPP
#define COMMON_SYSTEM_CONCURRENCY_LITE_RWLOCK_HPP
#pragma once

#include <assert.h>

#include "common/base/uncopyable.hpp"
#include "common/system/concurrency/atomic/atomic.hpp"
#include "common/system/concurrency/scoped_locker.hpp"
#include "common/system/concurrency/this_thread.hpp"
#include "common/system/memory/barrier.hpp"

class LiteRWLock
{
    DECLARE_UNCOPYABLE(LiteRWLock);
public:
    typedef ScopedReaderLocker<LiteRWLock> ReaderLocker;
    typedef ScopedWriterLocker<LiteRWLock> WriterLocker;
    typedef ScopedTryReaderLocker<LiteRWLock> TryReaderLocker;
    typedef ScopedTryWriterLocker<LiteRWLock> TryWriterLocker;
public:
    LiteRWLock() : m_lock(0), m_last_owner_id(0) {}
    bool TryReaderLock()
    {
        int value = m_lock;
        while (value >= 0)
        {
            if (AtomicCompareExchange(m_lock, value, value + 1, value))
                return true;
        }

        // lock writed, it's not necessary to try again
        return false;
    }

    void ReaderLock()
    {
        while (!TryReaderLock())
            ThisThread::Sleep(0);
        m_last_owner_id = ThisThread::GetId();
    }

    void ReaderUnlock()
    {
        m_last_owner_id = 0;
        int value = m_lock;
        assert(value > 0);
        while (value > 0)
        {
            if (AtomicCompareExchange(m_lock, value, value - 1, value))
                return;
        }
    }

    bool TryWriterLock()
    {
        int old_value;
        return AtomicCompareExchange(m_lock, 0, -1, old_value);
    }

    void WriterLock()
    {
        while (!TryWriterLock())
            ThisThread::Sleep(0);
        m_last_owner_id = ThisThread::GetId();
    }

    void WriterUnlock()
    {
        assert(m_lock == -1);
        m_last_owner_id = 0;
        int old_value = -1;
        while (!AtomicCompareExchange(m_lock, -1, 0, old_value))
        {
            assert(old_value == -1);
        }
    }
public: // only for test and debug, should not be used in normal code logical
    bool IsReaderLocked() const
    {
        MemoryReadBarrier();
        return m_lock > 0;
    }

    bool IsWriterLocked() const
    {
        MemoryReadBarrier();
        return m_lock < 0;
    }

    bool IsLocked() const
    {
        MemoryReadBarrier();
        return m_lock != 0;
    }

private:
    int m_lock;
    int m_last_owner_id;
};

#endif // COMMON_SYSTEM_CONCURRENCY_LITE_RWLOCK_HPP
