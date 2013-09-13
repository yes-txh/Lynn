// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 06/22/11
// Description:

#include "common/base/container_of.h"
#include "gtest/gtest.h"

struct Container
{
    int member1;
    int member2;
};

TEST(ContainerOf, Test)
{
    Container c;
    Container* pc = &c;
    int* p1 = &pc->member1;
    int* p2 = &pc->member2;
    EXPECT_EQ(pc, container_of(p1, Container, member1));
    EXPECT_EQ(pc, container_of(p2, Container, member2));
}
