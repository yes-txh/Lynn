// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_CRYPTO_RANDOM_TRUE_RANDOM_HPP
#define COMMON_CRYPTO_RANDOM_TRUE_RANDOM_HPP

#include <stddef.h>
#include <stdlib.h>
#include "common/base/stdint.h"
#include "common/base/uncopyable.hpp"

/// true random generator
class TrueRandom : Uncopyable
{
public:
    TrueRandom();
    ~TrueRandom();

    /// return random integer in range [0, UINT_MAX]
    uint32_t NextUInt32();

    /// return random integer in range [0, max_value)
    uint32_t NextUInt32(uint32_t max_value);

    /// return double in range [0.0,1.0]
    double NextDouble();

    /// generate random bytes
    bool NextBytes(void* buffer, size_t size);

private:
    union
    {
        int m_fd;               /// fd for /dev/urandom
        intptr_t m_hcrypt_prov; /// HCRYPTPROV
    };
};

#endif // COMMON_CRYPTO_RANDOM_TRUE_RANDOM_HPP

