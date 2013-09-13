// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/04/11
// Description: compatible unistd.h

#ifndef COMMON_BASE_COMPATIBLE_UNISTD_H
#define COMMON_BASE_COMPATIBLE_UNISTD_H
#pragma once

#include "common/base/compatible/internal.h"

#ifndef __unix__
# ifdef _MSC_VER
#  include <process.h>
#  include <stddef.h>

typedef int pid_t;
typedef intptr_t ssize_t;

COMPATIBLE_INLINE pid_t getpid()
{
    return _getpid();
}

# else
#  error unknown platform
# endif // _MSC_VER
#else
# include <unistd.h>
#endif // __unix__

#endif // COMMON_BASE_COMPATIBLE_UNISTD_H
