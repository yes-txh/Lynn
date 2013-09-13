// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/19/11
// Description:

#include "common/base/module.hpp"
#include "common/base/module/module_a.hpp"
#include "common/base/module/module_b.hpp"
#include "glog/logging.h"
#include "gtest/gtest.h"

TEST(Module, Test)
{
    EXPECT_EQ(1, GetA());
    EXPECT_EQ(2, GetB());
}

int main(int argc, char** argv)
{
    InitAllModulesAndTest(&argc, &argv);
    return RUN_ALL_TESTS();
}
