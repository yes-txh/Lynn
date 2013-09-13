#include "common/system/io/path.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "common/system/io/directory.hpp"

using namespace std;
using namespace io;

TEST(PathTest, Ops)
{
    string filepath = "io_test";
    string fullpath = directory::GetCurrentDir() + "/" + filepath;
    ASSERT_EQ(path::GetFullPath(filepath), fullpath);

    ASSERT_EQ(path::GetBaseName(filepath),  filepath);
    ASSERT_EQ(path::GetExtension(filepath), "");
    ASSERT_EQ(path::GetDirectory(filepath), "");

    filepath = "/";
    ASSERT_EQ(path::GetBaseName(filepath),  "");
    ASSERT_EQ(path::GetExtension(filepath), "");
    ASSERT_EQ(path::GetDirectory(filepath), "/");

    filepath = "////xxx//xx.x";
    ASSERT_EQ(path::GetBaseName(filepath),  "xx.x");
    ASSERT_EQ(path::GetExtension(filepath), ".x");
    ASSERT_EQ(path::GetDirectory(filepath), "////xxx//");
}


