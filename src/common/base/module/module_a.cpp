// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/18/11

#include <stdio.h>
#include "common/base/module.hpp"

int A;

DEFINE_MODULE(A)
{
    A = 1;
    return true;
}

DEFINE_MODULE_DTOR(A)
{
    printf("Module A dtor\n");
}

int GetA()
{
    return A;
}

