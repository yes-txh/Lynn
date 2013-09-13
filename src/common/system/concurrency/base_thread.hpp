// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_SYSTEM_CONCURRENCY_BASE_THREAD_HPP
#define COMMON_SYSTEM_CONCURRENCY_BASE_THREAD_HPP

#include <string>
#include "common/base/platform_features.hpp"
#include "common/system/concurrency/this_thread.hpp"
#include "common/system/concurrency/thread_types.hpp"

#undef Yield

/// BaseThread class is designed to be used as base class,
/// and override the Entry virtual method.
class BaseThread
{
public:
    typedef ThreadHandleType HandleType;
public:
    BaseThread();
    virtual ~BaseThread();
    void SetStackSize(size_t size);
    bool Start();

    // deprecated static methods: using ThisThread to instead please.
    DEPRECATED_BY(ThisThread::Exit) static void Exit();
    DEPRECATED_BY(ThisThread::Yield) static void Yield();
    DEPRECATED_BY(ThisThread::Sleep) static void Sleep(int time_in_ms);
    DEPRECATED_BY(ThisThread::GetLastErrorCode) static int GetLastErrorCode();
    DEPRECATED_BY(ThisThread::GetId) static int GetCurrentNativeId();

    bool Detach();
    bool Join();
    void SendStopRequest();
    bool IsStopRequested() const;
    bool StopAndWaitForExit();
    bool IsRunning() const;

    /// get handle
    HandleType GetHandle() const;

    /// get numeric thread id
    int GetId() const;

    static void SetDefaultStackSize(size_t size);

    static BaseThread* GetCurrent();

private:
    virtual void Entry() = 0;
    static void Initialize();
    static void DoInitialize();
    static void Cleanup(void* param);
private:
    static size_t s_DefaultStackSize;
#ifdef _WIN32
    static uint32_t s_nThreadStorageIndex;
    static unsigned int __stdcall StaticEntry(void* inThread);
#elif __unix__
    static pthread_once_t s_InitializeOnce;
    static pthread_key_t  s_nMainKey;
    static void* StaticEntry(void* inThread);
#endif

private:
    HandleType m_Handle;
    int m_Id;
    size_t m_StackSize;
    volatile bool m_bStopRequested;
    bool m_bJoined;
    volatile bool m_IsRunning;
};

inline void BaseThread::SendStopRequest()
{
    m_bStopRequested = true;
}

inline bool BaseThread::IsStopRequested() const
{
    return m_bStopRequested;
}

inline ThreadHandleType BaseThread::GetHandle() const
{
    return m_Handle;
}

inline void BaseThread::Exit()
{
    return ThisThread::Exit();
}

inline void BaseThread::Yield()
{
    return ThisThread::Yield();
}

inline void BaseThread::Sleep(int time_in_ms)
{
    return ThisThread::Sleep(time_in_ms);
}

inline int BaseThread::GetLastErrorCode()
{
    return ThisThread::GetLastErrorCode();
}

inline int BaseThread::GetCurrentNativeId()
{
    return ThisThread::GetId();
}

#endif // COMMON_SYSTEM_CONCURRENCY_BASE_THREAD_HPP

