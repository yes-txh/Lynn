// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/25/11
// Description: make duplicated module to test

#include "common/base/module.hpp"
#include "gtest/gtest.h"

bool Success()
{
    return true;
}

TEST(Module, DupTest)
{
    // register 2 duplicate module, the 2nd shoud be failed and death
    using ::base::internal::ModuleManager;
    ModuleManager::RegisterModuleCtor(__FILE__, __LINE__, "FOO", Success);
    EXPECT_DEATH(ModuleManager::RegisterModuleCtor(__FILE__, __LINE__, "FOO", Success),
                 "Duplicated");
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    return RUN_ALL_TESTS();
}
