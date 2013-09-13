// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/11/11
// Description: test URI class

#include "common/net/uri/uri.hpp"
#include "common/system/io/textfile.hpp"
#include "common/system/time/timestamp.hpp"
#include "gtest/gtest.h"

namespace net {

TEST(URI, Parse)
{
    std::string uristr = "http://www.baidu.com/s?tn=monline_dg&bs=DVLOG&f=8&wd=glog+DVLOG#fragment";
    URI uri;
    ASSERT_TRUE(uri.Parse(uristr));
    EXPECT_EQ(uristr, uri.ToString());
    EXPECT_EQ("http", uri.Scheme());

    ASSERT_EQ("/s", uri.Path());
    ASSERT_EQ("www.baidu.com", uri.Host());
    ASSERT_FALSE(uri.HasPort());

    ASSERT_TRUE(uri.HasQuery());
    EXPECT_EQ("tn=monline_dg&bs=DVLOG&f=8&wd=glog+DVLOG", uri.Query());

    ASSERT_TRUE(uri.HasFragment());
    ASSERT_EQ("fragment", uri.Fragment());
}

TEST(URI, BadUrl)
{
    URI uri;
    ASSERT_FALSE(uri.Parse("http://-www.lianjiew.com/")); // leading -
    ASSERT_FALSE(uri.Parse("http://platform_info.py/")); // domain contains _
    ASSERT_FALSE(uri.Parse(" http://platform-info.py/")); // leading space

    std::vector<std::string> urls;
    ASSERT_TRUE(io::textfile::ReadLines("baduris.txt", &urls));
    for (size_t i = 0; i < urls.size(); ++i)
        EXPECT_FALSE(uri.Parse(urls[i])) << urls[i];
}

class BatchTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        ASSERT_TRUE(io::textfile::ReadLines("uris.txt", &urls));
    }
protected:
    std::vector<std::string> urls;
};

TEST_F(BatchTest, Test)
{
    URI uri;

    uint64_t t = GetTimeStamp();
    const int kLoopCount = 100;
    for (int i = 0; i < kLoopCount; ++i)
    {
        for (size_t i = 0; i < urls.size(); ++i)
        {
            bool parse_result = uri.Parse(urls[i]);
            EXPECT_TRUE(parse_result) << urls[i];
            if (parse_result)
                EXPECT_EQ(urls[i], uri.ToString());
        }
    }
    printf("parsing %d uris in %d ms\n",
           kLoopCount * static_cast<int>(urls.size()),
           static_cast<int>(GetTimeStamp() - t));
}

}
