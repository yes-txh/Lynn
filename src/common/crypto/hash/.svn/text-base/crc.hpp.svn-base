// Copyright 2010, Tencent Inc.
// Author: Ivan Huang (ivanhuang@tencent.com)
//
// Defines checksum functions base on CRC algorithms.
// Refer to: http://en.wikipedia.org/wiki/Cyclic_redundancy_check

#ifndef COMMON_CRPYTO_HASH_CRC_H_
#define COMMON_CRPYTO_HASH_CRC_H_

#include <limits.h>
#include <string.h>
#include "common/base/stdint.h"

// CRC32算法
// 描述:
//       PKZip、WinZip 和 Ethernet 中的CRC算法,做了累进改进,改进见代码
// 备注:
//       适用于进行不定长累加的场合
//       实现的是CRC-32-IEEE 802.3算法，多项式为0x04C11DB7

static const uint32_t kCRC32InitValue = 0xffffffff;

uint32_t UpdateCRC32(const void* data, size_t size, uint32_t old_crc);

inline uint32_t CRC32Hash32(const void* data, size_t size) {
    return UpdateCRC32(data, size, kCRC32InitValue);
}

uint8_t  CRC16Hash8(const void* ptr, size_t size);

#endif // COMMON_CRPYTO_HASH_CRC_H_
