// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/25/11
// Description:

#include "common/base/module.hpp"

// an acycle module
DEFINE_MODULE(mod3)
{
    return true;
}
USING_MODULE(mod2);

