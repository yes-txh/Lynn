// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/25/11
// Description: test cycle dependancy

#include "common/base/module.hpp"
#include "gtest/gtest.h"

int g_argc;
char** g_argv;

TEST(Module, CycleTest)
{
    EXPECT_DEATH(InitAllModules(&g_argc, &g_argv), "mod1");
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    g_argc = argc;
    g_argv = argv;
    return RUN_ALL_TESTS();
}
