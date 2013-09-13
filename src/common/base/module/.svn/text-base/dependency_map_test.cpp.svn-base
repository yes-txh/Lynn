// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/24/11

#include "common/base/module/dependency_map.hpp"
#include "gtest/gtest.h"

namespace base
{

TEST(Dependency, Two)
{
    DependencyMap m;
    m["A"].insert("B");
    m["B"];
    std::vector<std::string> result;
    ASSERT_TRUE(TopologicalSort(m, &result));
    ASSERT_EQ("A", result[0]);
    ASSERT_EQ("B", result[1]);
}

TEST(Dependency, Three)
{
    DependencyMap m;
    m["A"].insert("B");
    m["B"].insert("C");
    m["C"];
    std::vector<std::string> result;
    ASSERT_TRUE(TopologicalSort(m, &result));
    ASSERT_EQ("A", result[0]);
    ASSERT_EQ("B", result[1]);
    ASSERT_EQ("C", result[2]);
}

} // end namespace base

