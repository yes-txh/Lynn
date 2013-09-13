// Copyright 2011, Tencent Inc.
// Author: Xiaokang Liu (hsiaokangliu@tencent.com)

#ifndef COMMON_SYSTEM_MEMORY_MEM_USAGE_H
#define COMMON_SYSTEM_MEMORY_MEM_USAGE_H

#include "common/base/stdint.h"

/// @brief Get memory usage of a specified process.
/// @param pid process id
/// @param vm_size virtual memory size (bytes)
/// @param mem_size pysical memory size (bytes)
/// @retval true if function runs successfully
bool GetMemUsage(int32_t pid, uint64_t* vm_size, uint64_t* mem_size);

#endif // COMMON_SYSTEM_MEMORY_MEM_USAGE_H
