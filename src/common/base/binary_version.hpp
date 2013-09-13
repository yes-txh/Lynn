// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/05/11
// Description: binary version information

#ifndef COMMON_BASE_BINARY_VERSION_HPP
#define COMMON_BASE_BINARY_VERSION_HPP
#pragma once

#include "common/base/platform_features.hpp"

extern "C" {
namespace binary_version {
WEAK_SYMBOL extern const char kBuildTime[];
WEAK_SYMBOL extern const char kBuilderName[];
WEAK_SYMBOL extern const char kHostName[];
WEAK_SYMBOL extern const char kCompiler[];
}
}
#endif // COMMON_BASE_BINARY_VERSION_HPP
