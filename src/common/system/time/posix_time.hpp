// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_SYSTEM_TIME_POSIX_TIME_HPP
#define COMMON_SYSTEM_TIME_POSIX_TIME_HPP

#ifdef __unix__

#include "common/base/stdint.h"

// for any timed* functions using absolute timespec
void RelativeTimeInMillSecondsToAbsTimeInTimeSpec(
    int64_t relative_time_in_ms,
    struct timespec* ts
);

#else
#error for POSIX compatible platforms only
#endif


#endif // COMMON_SYSTEM_TIME_POSIX_TIME_HPP
