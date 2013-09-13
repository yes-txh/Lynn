// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 04/30/11
// Description: string format

#ifndef COMMON_BASE_STRING_FORMAT_HPP
#define COMMON_BASE_STRING_FORMAT_HPP
#pragma once

#include <stddef.h>
#include <stdarg.h>
#include <string>
#include "common/base/platform_features.hpp"

size_t StringFormatAppendVA(std::string* dst, const char* format, va_list ap)
    __attribute__((format(printf, 2, 0)));
size_t StringFormatAppend(std::string* dst, const char* format, ...)
    __attribute__((format(printf, 2, 3)));
size_t StringFormatTo(std::string* dst, const char* format, ...)
    __attribute__((format(printf, 2, 3)));
std::string StringFormat(const char* format, ...)
    __attribute__((format(printf, 1, 2)));

#endif // COMMON_BASE_STRING_FORMAT_HPP
