// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_CRYPTO_RANDOM_PSEUDO_RANDOM_HPP
#define COMMON_CRYPTO_RANDOM_PSEUDO_RANDOM_HPP

#include <stddef.h>
#include "common/base/stdint.h"

/// α�����������������ͬ���㷨
class PseudoRandom
{
    /// can be copied safety
public:
    explicit PseudoRandom(uint64_t seed);

    /// return random integer between 0 ~ UINT32_MAX
    uint32_t NextUInt32();

    /** Returns the next random number, limited to a given range.
        @returns a random integer between 0 (inclusive) and maxValue (exclusive).
    */
    uint32_t NextUInt32(uint32_t max_value);

    /** Returns the next random floating-point number.
        @returns a random value in the range 0 to 1.0
    */
    double NextDouble();

    /** Resets this PseudoRandom object to a given seed value. */
    void SetSeed(uint64_t seed);

    /// ��������ֽ�����
    void NextBytes(void* buffer, size_t size);
private:
    uint64_t m_seed;
};

#endif // COMMON_CRYPTO_RANDOM_PSEUDO_RANDOM_HPP

