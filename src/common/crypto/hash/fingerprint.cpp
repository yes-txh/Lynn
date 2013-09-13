// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 04/14/2011 03:07:04 PM
// Description:

#include "common/crypto/hash/fingerprint.hpp"

uint64_t FingerPrint(const void *buf, size_t len) {
    const char* s = static_cast<const char*>(buf);
    const char* end = s + len;
    uint32_t high = 0, low = 0;
    for (; s < end; ++s) {
        low = (low << 5) - low + (*s);
        high = (high << 5) + high + (*s);
    }

    return (static_cast<uint64_t>(high) << 32) + low;
}

uint64_t FingerPrint(const char *s) {
    uint32_t high = 0, low = 0;
    for (; (*s) != '\0'; ++s) {
        low = (low << 5) - low + (*s);
        high = (high << 5) + high + (*s);
    }

    return (static_cast<uint64_t>(high) << 32) + low;
}

