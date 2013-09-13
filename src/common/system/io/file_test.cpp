#include "common/system/io/file.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include "common/base/compatible/stdlib.h"

using namespace std;
using namespace io;

TEST(FileTest, Ops)
{
    string original_file = "/bin/ls";
    string current_file = "/tmp/ls";
    file::Copy(original_file, current_file);
    ASSERT_TRUE(file::IsRegular(current_file));
    string bak_file = "/tmp/ls.bak";
    file::Copy(current_file, bak_file);
    ASSERT_TRUE(file::IsRegular(bak_file));
    file::Delete(bak_file);
    ASSERT_FALSE(file::IsRegular(bak_file));

    file::Rename(current_file, bak_file);
    ASSERT_TRUE(file::Exists(bak_file));
    ASSERT_FALSE(file::Exists(current_file));

    file::Rename(bak_file, current_file);
    ASSERT_TRUE(file::IsReadable(current_file));
    ASSERT_TRUE(file::IsWritable(current_file));
}

TEST(FileTest, Time)
{
    string original_file = "/bin/ls";
    string current_file = "/tmp/ls";
    file::Delete(current_file);
    file::Copy(original_file, current_file);
    time_t now = time(NULL);
    file::FileTime ft;
    ASSERT_TRUE(file::GetTime(current_file, &ft));
    EXPECT_LT(llabs(ft.create_time - now), 2);
    EXPECT_LT(llabs(ft.access_time - now), 2);
    EXPECT_LT(llabs(ft.modify_time - now), 2);

    sleep(2);
    file::Touch(current_file);
    now = time(NULL);
    ASSERT_TRUE(file::GetTime(current_file, &ft));
    EXPECT_LT(llabs(ft.create_time - now), 2);
    EXPECT_LT(llabs(ft.access_time - now), 2);
    EXPECT_LT(llabs(ft.modify_time - now), 2);

    file::Delete(current_file);
}

