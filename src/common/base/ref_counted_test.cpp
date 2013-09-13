// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/17/11

#include "common/base/ref_counted.hpp"
#include "common/base/scoped_refptr.hpp"
#include "gtest/gtest.h"

namespace {

class SelfAssign : public RefCountedBase<SelfAssign>
{
    friend class RefCountedBase<SelfAssign>;
    ~SelfAssign() {}
};

}  // end namespace

TEST(RefCountedTest, Count)
{
    SelfAssign* p = new SelfAssign;
    EXPECT_EQ(1, p->AddRef());
    EXPECT_TRUE(p->IsUnique());

    EXPECT_EQ(2, p->AddRef());
    EXPECT_FALSE(p->IsUnique());

    EXPECT_FALSE(p->Release());

    EXPECT_TRUE(p->IsUnique());
    EXPECT_TRUE(p->Release());
}

