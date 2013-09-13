// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2010-06-18

#include "common/system/concurrency/sync_event.hpp"

#ifdef _WIN32
#include <stdlib.h>
#include "common/base/common_windows.h"
#include "common/system/concurrency/system_error.hpp"

SyncEvent::SyncEvent(bool manual_reset, bool init_state, int spin_count)
{
    m_hEvent = CreateEvent(NULL, manual_reset, false, NULL);
    CHECK_WINDOWS_ERROR(m_hEvent != NULL);
}

SyncEvent::~SyncEvent()
{
    CHECK_WINDOWS_ERROR(CloseHandle(m_hEvent));
    m_hEvent = NULL;
}

void SyncEvent::Wait()
{
    CHECK_WINDOWS_WAIT_ERROR(WaitForSingleObject(m_hEvent, INFINITE));
}

// Timeout in milliseconds.
bool SyncEvent::Wait(int timeout)
{
    return CHECK_WINDOWS_WAIT_ERROR(WaitForSingleObject(m_hEvent, timeout));
}

bool SyncEvent::TryWait()
{
    return Wait(0);
}

void SyncEvent::Set()
{
    CHECK_WINDOWS_ERROR(SetEvent(m_hEvent));
}

void SyncEvent::Reset()
{
    CHECK_WINDOWS_ERROR(ResetEvent(m_hEvent));
}

#else // _WIN32

#include <stdio.h>
#include "common/system/concurrency/atomic/atomic.h"
#include "common/system/system_information.h"

SyncEvent::SyncEvent(bool manual_reset, bool init_state, int spin_count):
    m_manual_reset(manual_reset),
    m_spin_count(spin_count),
    m_signaled(init_state)
{
}

SyncEvent::~SyncEvent()
{
}

bool SyncEvent::SpinWait()
{
    static bool is_smp = GetLogicalCpuNumber() > 1;
    if (!is_smp)
        return false;

    for (int i = 0; i < m_spin_count; ++i)
    {
        if (m_manual_reset)
        {
            if (AtomicGet(m_signaled))
            {
                return true;
            }
        }
        else
        {
            if (AtomicCompareExchange(m_signaled, true, false))
            {
                return true;
            }
        }
    }

    return false;
}

void SyncEvent::Wait()
{
    if (SpinWait())
        return;

    MutexLocker locker(m_mutex);
    while (!m_signaled)
        m_cond.Wait(m_mutex);

    if (!m_manual_reset)
        m_signaled = false;
}

bool SyncEvent::Wait(int timeout)
{
    if (SpinWait())
        return true;

    MutexLocker locker(m_mutex);
    if (!m_signaled)
        m_cond.Wait(m_mutex, timeout);

    if (!m_signaled) return false;

    if (!m_manual_reset)
        m_signaled = false;

    return true;
}

bool SyncEvent::TryWait()
{
    return Wait(0);
}

void SyncEvent::Set()
{
    {
        MutexLocker locker(m_mutex);
        m_signaled = true;
    }

    if (m_manual_reset)
        m_cond.Broadcast();
    else
        m_cond.Signal();
}

void SyncEvent::Reset()
{
    MutexLocker locker(m_mutex);
    m_signaled = false;
}

#endif // _WIN32
