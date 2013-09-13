// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2011年04月19日 00时30分57秒
// Description:

#include "gtest/gtest.h"
#include "common/config/xml/xml_configer.hpp"

TEST(XmlConfiger, Test)
{
    XMLConfiger configer(".");
    ASSERT_TRUE(configer.ParseFile("test.xml"));
}
