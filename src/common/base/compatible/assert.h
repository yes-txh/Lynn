// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/13/11
// Description: redefine assert in release mode

// NOTE:
// Should not apply inclusion guard idiom to assert.h,
// assert.h can be included multiple times with different NDEBUG definition
// see the source code of assert.h in the std library.

#include <assert.h>

// if NDEBUG is defined, assert(e) will be defined to static_cast<void>(e)
// but if e is a locale variable which is only used in assert, it will be
// unused to compiler, and warning will be issues.
// redefine it to fix this problem
#ifdef NDEBUG
#undef assert
#define assert(e) ((void)sizeof(e))
#endif
