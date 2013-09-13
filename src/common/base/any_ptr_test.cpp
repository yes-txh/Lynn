// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include <gtest/gtest.h>
#include "common/base/any_ptr.hpp"

TEST(AnyPtr, Delete)
{
    AnyPtr p(new int(0));
    p.Delete();
}

