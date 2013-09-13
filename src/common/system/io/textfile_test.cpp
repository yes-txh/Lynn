// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/06/11
// Description: test textfile

#include "common/system/io/textfile.hpp"
#include "gtest/gtest.h"

TEST(TextFile, LoadToString)
{
    std::string s;
    ASSERT_TRUE(io::textfile::LoadToString("test_unix.txt", &s));
    EXPECT_EQ(773U, s.length());

    ASSERT_TRUE(io::textfile::LoadToString("test_dos.txt", &s));
    EXPECT_EQ(794U, s.length());
}

TEST(TextFile, UnixReadLines)
{
    std::vector<std::string> lines;
    ASSERT_TRUE(io::textfile::ReadLines("test_unix.txt", &lines));
    EXPECT_EQ(21U, lines.size());
    EXPECT_EQ("Long, long ago there lived a king.", lines[0]);
}

TEST(TextFile, DosReadLines)
{
    std::vector<std::string> lines;
    ASSERT_TRUE(io::textfile::ReadLines("test_dos.txt", &lines));
    EXPECT_EQ(21U, lines.size());
    EXPECT_EQ("Long, long ago there lived a king.", lines[0]);
}
