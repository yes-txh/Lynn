// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/24/11
// Description:

#ifndef COMMON_BASE_MODULE_DEPENDENCY_MAP_HPP
#define COMMON_BASE_MODULE_DEPENDENCY_MAP_HPP
#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

namespace base
{
/// represent dependency rules, such as makefile rules:
/// A: B C
/// B: C
/// C:
typedef std::map<std::string, std::set<std::string> > DependencyMap;

/// Dependency map is a DAG(directed acyclic graph), do topological sort on the DAG
/// yield sorted result.
bool TopologicalSort(const DependencyMap& depends, std::vector<std::string>* result);
}

#endif // COMMON_BASE_MODULE_DEPENDENCY_MAP_HPP
