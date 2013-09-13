// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2011-06-12 20:15:45
// Description:

#ifndef COMMON_SYSTEM_DEBUG_HEX_DUMP_HPP
#define COMMON_SYSTEM_DEBUG_HEX_DUMP_HPP

/// @file
/// @details
/// example:
/// 0000: 23 69 6E 63 6C 75 64 65-20 3C 63 6F 6D 6D 6F 6E  #include <commo
/// 0010: 2F 73 79 73 74 65 6D 2F-64 65 62 75 67 2F 68 65  /system/debug/h
/// 0020: 78 5F 64 75 6D 70 2E 68-70 70 3E 0A 23 69 6E 63  x_dump.hpp>.#in
/// 0040: 74 2E 68 3E 0A 0A 69 6E-74 20 6D 61 69 6E 28 29  t.h>..int main(
/// 0050: 0A 7B 0A 20 20 20 20 48-65 78 44 75 6D 70 28 28  .{.    HexDump(
/// 0070: 2C 20 28 76 6F 69 64 2A-29 6D 61 69 6E 29 3B 0A  , (void*)main);

#include <stddef.h>
#include <stdio.h>

/// @brief dump memory in hex format.
/// @param fp FILE* object to be dumped to
/// @param buffer buffer address to be dumped
/// @param size buffer size
/// @param print_address whether printf buffer address
void HexDump(FILE* fp, const void* buffer, size_t size, bool print_address = false);

/// @brief dump memory in hex format.
/// @param fd fd to be dumped to
/// @param buffer buffer address to be dumped
/// @param size buffer size
/// @param print_address whether printf buffer address
void HexDump(int fd, const void* buffer, size_t size, bool print_address = false);

#endif // COMMON_SYSTEM_DEBUG_HEX_DUMP_HPP
