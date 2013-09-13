// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2010-06-18

#ifndef COMMON_SYSTEM_CONCURRENCY_SYNC_EVENT_HPP
#define COMMON_SYSTEM_CONCURRENCY_SYNC_EVENT_HPP

/// @brief Win32 Event synchronization object
/// @author phongchen

#ifndef _WIN32
#include "common/system/concurrency/condition_variable.hpp"
#include "common/system/concurrency/mutex.hpp"
#endif

/// Windows Event Object
class SyncEvent
{
public:
    /// @param manual_reset manual_reset or auto reset
    /// @param init_state initial state is signaled or non-signaled
    /// @param spin_count try spin count times to check signaled flag before wait.
    /// if the event may be signaled quiet soon after wait, proper spin count may
    /// reduce the latency and then promote the total performance.
    SyncEvent(
        bool manual_reset = false,
        bool init_state = false,
        int spin_count = 0
    );
    ~SyncEvent();
    void Wait();
    bool Wait(int timeout); // in ms
    bool TryWait();
    void Set();
    void Reset();
private:
    SyncEvent(const SyncEvent&);
    SyncEvent& operator=(const SyncEvent&);
    bool SpinWait();
private:
#ifdef _WIN32
    void* m_hEvent;
#else
    Mutex m_mutex;
    ConditionVariable m_cond;
    const bool m_manual_reset;
    const int m_spin_count;
    bool m_signaled;
#endif
};

#endif // COMMON_SYSTEM_CONCURRENCY_SYNC_EVENT_HPP
