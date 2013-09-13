// Copyright (c) 2011, Tencent Inc. All rights reserved.

#include "common/system/concurrency/atomic/atomic.h"
#include "common/system/concurrency/base_thread.hpp"
#include "common/system/concurrency/system_error.hpp"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t BaseThread::s_DefaultStackSize;

void BaseThread::SetStackSize(size_t size)
{
    m_StackSize = size;
}

BaseThread::BaseThread():
    m_Handle(),
    m_Id(-1),
    m_StackSize(s_DefaultStackSize),
    m_bStopRequested(false),
    m_bJoined(false),
    m_IsRunning(false)
{
    Initialize();
}

void BaseThread::SetDefaultStackSize(size_t size)
{
    s_DefaultStackSize = size;
}

#ifdef __unix__
///////////////////////////////////////////////////////////////////////////////
// Posix implementation

pthread_once_t BaseThread::s_InitializeOnce = PTHREAD_ONCE_INIT;
pthread_key_t BaseThread::s_nMainKey = 0;

void BaseThread::Initialize()
{
    CHECK_PTHREAD_ERROR(pthread_once(&s_InitializeOnce, DoInitialize));
}

void BaseThread::DoInitialize()
{
    CHECK_PTHREAD_ERROR(pthread_key_create(&BaseThread::s_nMainKey, NULL));
}

BaseThread::~BaseThread()
{
    if (IsRunning() && !m_bJoined)
        Join();
}

bool BaseThread::Start()
{
    m_Handle = ThreadHandleType();
    m_Id = 0;
    pthread_attr_t attr;
    CHECK_PTHREAD_ERROR(pthread_attr_init(&attr));
    if (m_StackSize)
        CHECK_PTHREAD_ERROR(pthread_attr_setstacksize(&attr, m_StackSize));
    m_bStopRequested = false;
    int error = pthread_create(&m_Handle, &attr, StaticEntry, this);
    if (error != 0 && error != EAGAIN)
        CHECK_PTHREAD_ERROR(error);
    CHECK_PTHREAD_ERROR(pthread_attr_destroy(&attr));
    return error == 0;
}

bool BaseThread::StopAndWaitForExit()
{
    m_bStopRequested = true;
    if (!m_bJoined)
        return Join();
    return false;
}

bool BaseThread::Detach()
{
    CHECK_PTHREAD_ERROR(pthread_detach(m_Handle));
    m_Handle = HandleType();
    return true;
}

bool BaseThread::Join()
{
    // What we're trying to do is allow the thread we want to delete to complete
    // running. So we wait for it to stop.
    assert(!m_bJoined);
    m_bJoined = true;
    const pthread_t null_thread_id = pthread_t();
    assert(!pthread_equal(m_Handle, null_thread_id));
    CHECK_PTHREAD_ERROR(pthread_join(m_Handle, NULL));
    m_Handle = null_thread_id;
    m_Id = -1;
    return true;
}

int BaseThread::GetId() const
{
    if (m_Id != 0)
        return m_Id;

    // GetId is rarely used, so busy wait is more fitness
    while (AtomicGet(m_Id) == 0)
        ThisThread::Sleep(1);

    return m_Id;
}

bool BaseThread::IsRunning() const
{
    return m_IsRunning;
}

// make sure execute before exit
void BaseThread::Cleanup(void* param)
{
    BaseThread* thread = static_cast<BaseThread*>(param);
    thread->m_IsRunning = false;
}

void* BaseThread::StaticEntry(void* inBaseThread)
{
    BaseThread* theBaseThread = static_cast<BaseThread*>(inBaseThread);
    theBaseThread->m_IsRunning = true;
    theBaseThread->m_Id = ThisThread::GetId();
    pthread_setspecific(BaseThread::s_nMainKey, theBaseThread);

    pthread_cleanup_push(Cleanup, inBaseThread);
    theBaseThread->Entry();
    theBaseThread->m_IsRunning = false;
    pthread_cleanup_pop(false);

    return 0;
}

BaseThread* BaseThread::GetCurrent()
{
    return static_cast<BaseThread *>(pthread_getspecific(BaseThread::s_nMainKey));
}

#endif // __unix__

//////////////////////////////////////////////////////////////////////////////
// Windows Implementation
#ifdef  _WIN32
#include <process.h>
#include "common/base/common_windows.h"

uint32_t BaseThread::s_nThreadStorageIndex = 0;

void BaseThread::Initialize()
{
    static int s_InitializeOnce = (DoInitialize(), 1);
    (void) s_InitializeOnce;
}

void BaseThread::DoInitialize()
{
    s_nThreadStorageIndex = ::TlsAlloc();
    assert(s_nThreadStorageIndex >= 0);
}

BaseThread::~BaseThread()
{
    CloseHandle(m_Handle);
}

bool BaseThread::Start()
{
    unsigned int tid = 0;
    m_bStopRequested = false;
    m_Handle = (HANDLE)_beginthreadex(NULL, s_DefaultStackSize, StaticEntry, this, 0, &tid);
    m_Id = tid;
    return m_Handle != NULL;
}

bool BaseThread::StopAndWaitForExit()
{
    m_bStopRequested = true;
    if (!m_bJoined)
        return Join();
    return false;
}

bool BaseThread::Detach()
{
    return true;
}

bool BaseThread::Join()
{
    // What we're trying to do is allow the thread we want to delete to complete
    // running. So we wait for it to stop.
    assert(!m_bJoined);
    m_bJoined = true;
    if (m_Handle)
    {
        DWORD error_code = ::WaitForSingleObject(m_Handle, INFINITE);
        return error_code == WAIT_OBJECT_0;
    }
    return false;
}

int BaseThread::GetId() const
{
    return m_Id;
}


bool BaseThread::IsRunning() const
{
    return WaitForSingleObject(m_Handle, 0) == WAIT_TIMEOUT;
}

unsigned int __stdcall BaseThread::StaticEntry(void* inBaseThread)
{
    BaseThread* theBaseThread = static_cast<BaseThread*>(inBaseThread);
    ::TlsSetValue(s_nThreadStorageIndex, theBaseThread);
    theBaseThread->Entry();
    return 0;
}

BaseThread* BaseThread::GetCurrent()
{
    return static_cast<BaseThread *>(::TlsGetValue(s_nThreadStorageIndex));
}

#endif // _WIN32
