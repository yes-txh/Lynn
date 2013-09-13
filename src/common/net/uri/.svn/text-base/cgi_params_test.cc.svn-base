// Copyright (c) 2011, Tencent.com
// All rights reserved.

/// @file cgi_params_test.cc
/// @brief cgi param test
/// @date  03/30/2011 11:05:46 PM
/// @author CHEN Feng <phongchen@tencent.com>

#include "common/net/uri/cgi_params.hpp"
#include "gtest/gtest.h"

TEST(CgiParams, Parse)
{
    CgiParams params;
    ASSERT_TRUE(params.Parse("a=1&b=2&c=3&d"));
    ASSERT_EQ(4U, params.Count());
    ASSERT_EQ("a", params.Get(0).name);
    ASSERT_EQ("1", params.Get(0).value);

    ASSERT_EQ("b", params.Get(1).name);
    ASSERT_EQ("2", params.Get(1).value);

    ASSERT_EQ("c", params.Get(2).name);
    ASSERT_EQ("3", params.Get(2).value);

    ASSERT_EQ("d", params.Get(3).name);
    ASSERT_EQ("", params.Get(3).value);
}
