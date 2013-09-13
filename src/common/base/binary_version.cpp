// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/23/11
// Description:

#include "common/base/binary_version.hpp"
#include "common/base/platform_features.hpp"

extern "C" {
namespace binary_version {
WEAK_SYMBOL extern const char kBuildTime[] = "Unknown";
WEAK_SYMBOL extern const char kBuilderName[] = "Unknown";
WEAK_SYMBOL extern const char kHostName[] = "Unknown";
WEAK_SYMBOL extern const char kCompiler[] = "Unknown";
}
}

