#include "common/web/url.hpp"
#include <gtest/gtest.h>

TEST(Url, Parse)
{
    web::url::Url url;
    std::string str_url1 = "http://www.a.com";
    std::string str_url2 = "http://www.b.com";
    EXPECT_TRUE(url.Load(str_url1.c_str(), str_url1.size()));
    EXPECT_TRUE(url.Load(str_url2.c_str(), str_url2.size()));
    str_url2 = "http://127.0.0.1";
    EXPECT_TRUE(url.Load(str_url2.c_str(), str_url2.size())) << str_url2;
    str_url2 = "http://0.0.0.0";
    EXPECT_TRUE(url.Load(str_url2.c_str(), str_url2.size())) << str_url2;
    str_url2 = "http://0.0.0.1";
    EXPECT_TRUE(url.Load(str_url2.c_str(), str_url2.size())) << str_url2;
    str_url2 = "http://123.123.0.1";
    EXPECT_TRUE(url.Load(str_url2.c_str(), str_url2.size())) << str_url2;
}

int main(int argc, char ** argv)
{
   testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

