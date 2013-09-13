#include <gtest/gtest.h>
#include "common/system/dynamic_library.hpp"

TEST(DynamicLibrary, LoadAndUnload)
{
    DynamicLibrary dll;
    ASSERT_TRUE(dll.Load("libm.so"));
    ASSERT_TRUE(dll.Load("libm.so"));
    ASSERT_TRUE(dll.Unload());
    ASSERT_FALSE(dll.IsLoaded());
}

TEST(DynamicLibrary, GetSymbol)
{
    DynamicLibrary dll;
    ASSERT_TRUE(dll.Load("libm.so"));
    double (*log10)(double x);
    ASSERT_TRUE(dll.GetSymbol("log10", &log10));
    ASSERT_DOUBLE_EQ(log10(10.0), 1.0);
}

