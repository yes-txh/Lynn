// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/system/concurrency/semaphore.hpp"
#include <stdlib.h>

#ifdef _WIN32

#include "common/base/common_windows.h"
#include <winnt.h>

//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

//
// semaphore object query structure
//
typedef struct _SEMAINFO {
    UINT        Count;      // current semaphore count
    UINT        Limit;      // max semaphore count
} SEMAINFO, *PSEMAINFO;

//
// only one query type for semaphores
//
#define SEMAQUERYINFOCLASS  0

typedef LONG NTSTATUS;

//
// NT Function calls.
//
EXTERN_C NTSTATUS
WINAPI
NtQuerySemaphore(
    HANDLE Handle,
    UINT InfoClass,
    PSEMAINFO SemaInfo,
    UINT InfoSize,
    PUINT RetLen
);

static BOOL WINAPI QuerySemaphore(
    HANDLE  hSemaphore,
    LPLONG  lpCount
)
{
    SEMAINFO            SemInfo;
    UINT                RetLen;
    NTSTATUS Status = NtQuerySemaphore(
        hSemaphore,
        SEMAQUERYINFOCLASS,
        &SemInfo,
        sizeof SemInfo,
        &RetLen
    );

    if (!NT_SUCCESS(Status))
    {
        *lpCount = -1;
        return FALSE;
    }
    else
    {
        *lpCount = SemInfo.Count;
        return TRUE;
    }
}

Semaphore::Semaphore(unsigned int value)
{
    m_hSemaphore = CreateSemaphore(NULL, value, INT_MAX, NULL);
    if (m_hSemaphore == NULL)
        HandleError();
}

Semaphore::~Semaphore()
{
    Checked(CloseHandle(m_hSemaphore));
    m_hSemaphore = NULL;
}

void Semaphore::Wait()
{
    DWORD result = WaitForSingleObject(m_hSemaphore, INFINITE);
    if (result != WAIT_OBJECT_0)
        HandleError();
}

bool Semaphore::TimedWait(int timeout) // in ms
{
    if (timeout < 0)
    {
        Wait();
        return true;
    }

    DWORD result = WaitForSingleObject(m_hSemaphore, timeout);
    switch (result)
    {
    case WAIT_OBJECT_0:
        return true;
    case WAIT_TIMEOUT:
        return false;
    default:
        HandleError();
        break;
    }
    return false;
}

bool Semaphore::TryWait()
{
    return TimedWait(0);
}

void Semaphore::Release()
{
    Checked(ReleaseSemaphore(m_hSemaphore, 1, NULL));
}

unsigned int Semaphore::GetValue() const
{
    LONG value;
    Checked(QuerySemaphore(m_hSemaphore, &value));
    return value;
}

void Semaphore::Checked(int success)
{
    if (!success)
    {
        HandleError();
    }
}

void Semaphore::HandleError()
{
    abort();
}

#endif // _WIN32

