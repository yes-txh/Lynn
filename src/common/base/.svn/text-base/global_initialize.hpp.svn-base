// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2011年04月21日 01时37分13秒
// Description: global initialize

#ifndef COMMON_BASE_GLOBAL_INITIALIZE_HPP
#define COMMON_BASE_GLOBAL_INITIALIZE_HPP

/// global initialize, run some code before main
/// @param name module name
#define GLOBAL_INITIALIZE(name) \
static void name##_init(); \
static bool call_global_##name##_init() { name##_init(); return true; } \
bool name##_initialized = call_global_##name##_init(); \
static void name##_init()

#if 0

// example
GLOBAL_INITIALIZE(this_test) {
    FLAGS_gtest_death_test_style = "threadsafe";
    print("test initialized");
}

#endif

#endif // COMMON_BASE_GLOBAL_INITIALIZE_HPP
