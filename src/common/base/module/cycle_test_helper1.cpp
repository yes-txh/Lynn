// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/25/11

#include "common/base/module.hpp"

DEFINE_MODULE(mod1)
{
    return true;
}
USING_MODULE(mod2);
