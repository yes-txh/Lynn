// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_COMMON_WINDOWS_H
#define COMMON_BASE_COMMON_WINDOWS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501 // windows xp
#endif

#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#include <windows.h>

#endif // COMMON_BASE_COMMON_WINDOWS_H
