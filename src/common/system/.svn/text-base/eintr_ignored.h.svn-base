// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 06/23/11
// Description:

#ifndef COMMON_SYSTEM_EINTR_IGNORED_H
#define COMMON_SYSTEM_EINTR_IGNORED_H
#pragma once

#include "common/base/compatible/errno.h"

#ifdef __unix__
#define EINTR_IGNORED(expr) \
    ({ \
        __typeof__(expr) result; \
        do { result = (expr); } while (result < 0 && errno == EINTR); \
        result; \
    })
#else
#define EINTR_IGNORED(expr) (expr)
#endif

#endif // COMMON_SYSTEM_EINTR_IGNORED_H
