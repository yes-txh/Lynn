// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 06/23/11
// Description:

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/system/concurrency/system_error.hpp"

#ifdef _WIN32

#include "common/base/common_windows.h"

static void ReportError(const char* function_name, DWORD error_code)
{
    abort();
}

void CheckWindowsError(const char* function_name, bool error)
{
    if (!error)
        ReportError(function_name, ::GetLastError());
}

bool CheckWindowsWaitError(const char* function_name, unsigned int code)
{
    switch (code)
    {
    case WAIT_TIMEOUT:
        return false;
    case WAIT_FAILED:
        ReportError(function_name, ::GetLastError());
    }
    return true;
}

void CheckNtError(const char* function_name, long ntstatus)
{
    if (ntstatus)
        ReportError(function_name, ntstatus);
}

#endif

#ifdef __unix__

static void HandleErrnoError(const char* function_name, int error)
{
    const char* msg = strerror(error);
    fprintf(stderr, "%s: Fatal error, %s", function_name, msg);
    abort();
}

void CheckErrnoError(const char* function_name, int error)
{
    if (error)
        HandleErrnoError(function_name, error);
}

void CheckPosixError(const char* function_name, int result)
{
    if (result < 0)
        HandleErrnoError(function_name, errno);
}

bool CheckPosixTimedError(const char* function_name, int result)
{
    if (result < 0)
    {
        int error = errno;
        if (error == ETIMEDOUT)
            return false;
        HandleErrnoError(function_name, error);
    }

    return true;
}

bool CheckPthreadTimedError(const char* function_name, int error)
{
    if (error == 0)
        return true;
    if (error == ETIMEDOUT)
        return false;
    HandleErrnoError(function_name, error);
    return false;
}

bool CheckPthreadTryLockError(const char* function_name, int error)
{
    if (error == 0)
        return true;
    if (error == EBUSY || error == EAGAIN)
        return false;
    HandleErrnoError(function_name, error);
    return false;
}

#endif // __unix__
