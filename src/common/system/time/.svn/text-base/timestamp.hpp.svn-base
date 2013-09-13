// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_SYSTEM_TIME_TIMESTAMP_HPP
#define COMMON_SYSTEM_TIME_TIMESTAMP_HPP

#include "common/base/platform_features.hpp"
#include "common/base/stdint.h"

/// time stamp in millisecond (1/1000 second)
int64_t GetTimeStampInMs();

DEPRECATED_BY(GetTimeStampInMs)
inline int64_t GetTimeStamp()
{
    return GetTimeStampInMs();
}

/// time stamp in microsecond (1/1000000 second)
int64_t GetTimeStampInUs();

#endif // COMMON_SYSTEM_TIME_TIMESTAMP_HPP
