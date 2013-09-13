// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 06/23/11
// Description: common pthread error handler

#ifndef COMMON_SYSTEM_CONCURRENCY_SYSTEM_ERROR_HPP
#define COMMON_SYSTEM_CONCURRENCY_SYSTEM_ERROR_HPP
#pragma once

#ifdef _WIN32

void CheckWindowsError(const char* function_name, bool error);
bool CheckWindowsWaitError(const char* function_name, unsigned int code);
void CheckNtError(const char* function_name, long ntstatus);

#define CHECK_WINDOWS_ERROR(bool_value) \
    CheckWindowsError(__FUNCTION__, bool_value)

#define CHECK_WINDOWS_WAIT_ERROR(expr) \
    CheckWindowsWaitError(__FUNCTION__, expr)

#define CHECK_NT_ERROR(expr) \
    CheckNtError(__FUNCTION__, expr)

#endif

#ifdef __unix__

void CheckErrnoError(const char* function_name, int error);
void CheckPosixError(const char* function_name, int result);
bool CheckPosixTimedError(const char* function_name, int error);
bool CheckPthreadTimedError(const char* function_name, int error);
bool CheckPthreadTryLockError(const char* function_name, int error);

#define CHECK_ERRNO_ERROR(expr) \
    CheckErrnoError(__PRETTY_FUNCTION__, (expr))

#define CHECK_POSIX_ERROR(expr) \
    CheckPosixError(__PRETTY_FUNCTION__, (expr))

#define CHECK_POSIX_TIMED_ERROR(expr) \
    CheckPosixTimedError(__PRETTY_FUNCTION__, (expr))

#define CHECK_PTHREAD_ERROR(expr) \
    CHECK_ERRNO_ERROR((expr))

#define CHECK_PTHREAD_TIMED_ERROR(expr) \
    CheckPthreadTimedError(__PRETTY_FUNCTION__, (expr))

#define CHECK_PTHREAD_TRYLOCK_ERROR(expr) \
    CheckPthreadTryLockError(__PRETTY_FUNCTION__, (expr))

#endif

#endif // COMMON_SYSTEM_CONCURRENCY_SYSTEM_ERROR_HPP
