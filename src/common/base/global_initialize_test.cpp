// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2011年04月21日 01时41分21秒
// Description:

#include "common/base/global_initialize.hpp"
#include "gtest/gtest.h"

static int global_int = 0;

GLOBAL_INITIALIZE(global_initialize_test)
{
    global_int = 1;
}

TEST(GlibalInit, Test)
{
    EXPECT_EQ(1, global_int);
}
