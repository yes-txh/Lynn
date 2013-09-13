// Copyright (c) 2011, Tencent.com
// All rights reserved.

/// @file malloc.hpp
/// @brief malloc related functions
/// @date  04/12/2011 04:31:25 PM
/// @author CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_COMPATIBLE_MALLOC_H
#define COMMON_BASE_COMPATIBLE_MALLOC_H

#include <stdlib.h>
#include <malloc.h>

/// return mallocated black size, may be larger than malloc required
inline size_t malloced_size(void* p)
{
#if defined _MSC_VER
    return _msize(p);
#elif defined __GNUC__
    return malloc_usable_size(p);
#else
#error Unknown platform
#endif
}

/// Allocate aligned memory
inline void* aligned_malloc(size_t size, size_t align)
{
#if defined _MSC_VER
    return _aligned_malloc(size, align);
#elif defined __unix__
    void* p = NULL;
    posix_memalign(&p, align, size);
    return p;
#else
#error Unknown platform
#endif
}

/// free aligned memory
inline void aligned_free(void* p)
{
#if defined _MSC_VER
    _unaligned_free(p);
#elif defined __GNUC__
    free(p);
#else
#error Unknown platform
#endif
}

#endif // COMMON_BASE_COMPATIBLE_MALLOC_H
