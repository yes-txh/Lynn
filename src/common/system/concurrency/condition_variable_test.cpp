// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#include "common/system/concurrency/condition_variable.hpp"
#include <gtest/gtest.h>

TEST(ConditionVariable, Init)
{
    ConditionVariable cond;
}

TEST(ConditionVariable, Wait)
{
    ConditionVariable event;
    event.Signal();
}

TEST(ConditionVariable, Release)
{
}

