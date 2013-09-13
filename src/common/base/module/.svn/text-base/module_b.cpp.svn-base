// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/18/11

#include <stdio.h>
#include "common/base/module.hpp"
#include "common/base/module/module_a.hpp"

int B;
DEFINE_MODULE(B)
{
    B = GetA() + 1;
    return true;
}

DEFINE_MODULE_DTOR(B)
{
    printf("Module B dtor\n");
}

USING_MODULE(A);


int GetB()
{
    return B;
}

