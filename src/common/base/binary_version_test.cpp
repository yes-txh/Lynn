// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/05/11
// Description: test binary version

#include "common/base/binary_version.hpp"
#include "gtest/gtest.h"

TEST(BinaryVersion, Test)
{
    printf("build time: %s\n", binary_version::kBuildTime);
    printf("builder name: %s\n", binary_version::kBuilderName);
    printf("host name: %s\n", binary_version::kHostName);
    printf("compiler: %s\n", binary_version::kCompiler);
}
