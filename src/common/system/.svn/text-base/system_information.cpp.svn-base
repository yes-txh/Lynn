// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2010-06-21

#include "common/system/system_information.h"
#include <stdlib.h>

#ifdef __unix__

#include <unistd.h>
#include <sys/user.h>

unsigned int GetLogicalCpuNumber()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

unsigned long long GetPhysicalMemorySize()
{
    return sysconf(_SC_PHYS_PAGES) * (unsigned long long) PAGE_SIZE;
}

#elif defined _WIN32

#include <common/base/common_windows.h>

unsigned int GetLogicalCpuNumber()
{
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    return SystemInfo.dwNumberOfProcessors;
}

unsigned long long GetPhysicalMemorySize()
{
    MEMORYSTATUSEX memory_status = { sizeof(memory_status) };
    GlobalMemoryStatusEx(&memory_status);
    return memory_status.ullTotalPhys;
}

#endif

std::string GetUserName()
{
    const char* username = getenv("USER");
    return username != NULL ? username : getenv("USERNAME");
}

