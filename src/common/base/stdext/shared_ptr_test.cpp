// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 04/15/2011 12:42:32 PM
// Description: shared ptr test

#include "shared_ptr.hpp"
#include "gtest/gtest.h"

TEST(SharedPtr, Test)
{
    stdext::shared_ptr<int> p(new int());
    stdext::shared_ptr<int> q = p;
}

class Foo : public stdext::enable_shared_from_this<Foo>
{
};

TEST(SharedPtr, This)
{
    stdext::shared_ptr<Foo> p(new Foo());
    stdext::shared_ptr<Foo> q = p;
}
