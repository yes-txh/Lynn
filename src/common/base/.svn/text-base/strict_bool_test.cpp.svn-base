// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/26/11
// Description:

#include "common/base/strict_bool.hpp"
#include "gtest/gtest.h"

TEST(StrictBool, CompileTest)
{
    strict_bool b;
    strict_bool b2(true);
    strict_bool b3(false);
    bool bb = b; bb = true;
    b == b2;
    b != b2;
    b = true;
    b = false;
    true == b;
    false == b;
    b == true;
    b == false;
    b != true;
    b != false;
    false != b;
    true != b;
}

// all following code should not compile
TEST(StrictBool, NoCompileTest)
{
    strict_bool b;
    // strict_bool b0(0);
    // strict_bool b1(1);
    // b = 1;
    // b = 0;
    // int a = b; a = 0;
    // b == 1;
    // b == 0;
    // b != 1;
    // b != 0;
    // 1 == b;
    // 0 == b;
    // 1 != b;
    // 0 != b;
}
