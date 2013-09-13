// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/crypto/random/pseudo_random.hpp"

#include <stdio.h>
#include <time.h>

#include "gtest/gtest.h"

TEST(PseudoRandom, Test)
{
    PseudoRandom tr(time(NULL));
    printf("%u\n", tr.NextUInt32());
    printf("%g\n", tr.NextDouble());
}
