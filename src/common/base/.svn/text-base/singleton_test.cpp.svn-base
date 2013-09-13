// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 04/29/11
// Description: test singleton

#include "common/base/singleton.hpp"
#include "gtest/gtest.h"

// make a singleton class
class TestClass : public SingletonBase<TestClass>
{
    friend class SingletonBase<TestClass>;
private:
    TestClass(){}
public:
    int Test() const
    {
        return 1;
    }
};

TEST(Singleton, SingletonClass)
{
    TestClass& test = TestClass::Instance();
    EXPECT_EQ(1, test.Test());
}

class TestClass2 {};
TEST(Singleton, ClassSingleton)
{
    TestClass2& test = Singleton<TestClass2>::Instance();
    (void) test;
}

