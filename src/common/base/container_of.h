// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 06/22/11
// Description:

#ifndef COMMON_BASE_CONTAINER_OF_H
#define COMMON_BASE_CONTAINER_OF_H
#pragma once

#include <stddef.h>


/**
 * @macro container_of
 * @brief cast a member of a structure out to the containing structure
 * @param ptr the pointer to the member.
 * @param type the type of the container struct this is embedded in.
 * @param member the name of the member within the struct.
 */

#ifndef container_of
#ifdef __GNUC__
#define container_of(ptr, type, member) \
    ({ \
        const typeof( ((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - offsetof(type, member) ); \
    })
#else
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
#endif
#endif // container_of

#endif // COMMON_BASE_CONTAINER_OF_H
