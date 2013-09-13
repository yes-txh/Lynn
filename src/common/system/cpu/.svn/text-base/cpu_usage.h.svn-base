// Copyright 2011, Tencent Inc.
// Author: Xiaokang Liu (hsiaokangliu@tencent.com)

#ifndef COMMON_SYSTEM_CPU_CPU_USAGE_H
#define COMMON_SYSTEM_CPU_CPU_USAGE_H

#include "common/base/stdint.h"

/// @brief Get cpu usage of a specified process.
/// This function is implemented simply on windows platform.
/// On windows os, it's not reenterable.
/// @param pid process id
/// @param cpu cpu usage
/// @retval true if function runs succesfully
bool GetCpuUsage(int32_t pid, double* cpu);

#endif // COMMON_SYSTEM_CPU_CPU_USAGE_H
