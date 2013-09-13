// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 04/30/11
// Description: test string concat

#include "common/base/string/concat.hpp"
#include "common/base/string/format.hpp"
#include "common/base/string/concat_test.pb.h"
#include "protobuf/text_format.h"
#include "gtest/gtest.h"
#include "perftools/profiler.h"

TEST(StringConcat, Concat)
{
    EXPECT_EQ("helloworld", StringConcat("hello", "world"));
    EXPECT_EQ("hello,world", StringConcat("hello", ",", "world"));
    EXPECT_EQ("xxx1024", StringConcat("xxx", 1024));
    EXPECT_EQ("xxx,1024", StringConcat("xxx", ",", 1024));
    EXPECT_EQ("xxx1024-1024", StringConcat("xxx", 1024, -1024));
    EXPECT_EQ("xxx1", StringConcat("xxx", static_cast<uint16_t>(1)));
    EXPECT_EQ("0123456789ABCDEF",
              StringConcat(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, "A", "B", "C", "D", "E", 'F'));
}

TEST(StringConcat, Append)
{
    std::string s = "hello";
    StringAppend(&s, "world");
    EXPECT_EQ("helloworld", s);
}

const int kTestLoopCount = 500000;
TEST(StringConcat, ConcatPerformance)
{
    std::string s;
    for (int i = 0; i < kTestLoopCount; ++i)
        StringConcatTo(&s, i, i, i, i, i, i, i, i, i, i);
}

TEST(StringConcat, FormatPerformance)
{
    std::string s;
    for (int i = 0; i < kTestLoopCount; ++i)
        StringFormatTo(&s, "%d%d%d%d%d%d%d%d%d%d", i, i, i, i, i, i, i, i, i, i);
}

class PerformanceTest : public testing::Test {};

TEST_F(PerformanceTest, StringConcatTo)
{
    std::string result;
    for (int i = 0; i < kTestLoopCount; ++i)
    {
        StringConcatTo(&result,
                       "hello", ",", "world",
                       100000000,
                       200000000,
                       300000000,
                       400000000,
                       500000000);
    }
}

TEST_F(PerformanceTest, ProtoBuf)
{
    std::string result;
    common::base::string::concat::test::TestMessage msg;
    msg.set_s1("hello");
    msg.set_s2(",");
    msg.set_s3("world");
    msg.set_n4(100000000);
    msg.set_n5(200000000);
    msg.set_n6(300000000);
    msg.set_n7(400000000);
    msg.set_n8(500000000);
    msg.set_f9("\xFF\x95\x27");
    msg.mutable_m10()->set_s1("ABCD");
    msg.mutable_m10()->set_s2("ABCD");
    msg.mutable_m10()->add_n3(1);
    msg.mutable_m10()->add_n3(2);
    msg.mutable_m10()->add_n3(3);
    std::string str;
    protobuf::TextFormat::PrintToString(msg, &str);
    std::cout << str << std::endl;
    for (int i = 0; i < kTestLoopCount; ++i)
        msg.SerializeToString(&result);
}

int main(int argc, char** argv)
{
    ProfilerStart(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    int n = RUN_ALL_TESTS();
    ProfilerStop();
    return n;
}
