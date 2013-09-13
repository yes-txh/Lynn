#define USE_MD5 1

#include "common/web/domain.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace websearch::domain;

#define SUFFIX_SERVER_IP    "172.24.28.218"
#define SUFFIX_SERVER_PORT  50690

TEST(Domain, Initialize)
{
    EXPECT_TRUE(Domain::Initialize("domainsuffix.txt", "blogsuffix.txt", NULL));
}

TEST(Domain, GetDomainSuffix)
{
    EXPECT_TRUE(Domain::Initialize("domainsuffix.txt", NULL, NULL));
    {
        Domain domain("news.qq.com");
        ASSERT_EQ(std::string(domain.GetDomainSuffix()), std::string(".com"));
    }
    {
        Domain domain("news.sina.com.cn");
        ASSERT_EQ(std::string(domain.GetDomainSuffix()), std::string(".com.cn"));
    }
    {
        Domain domain("news.djq");
        ASSERT_TRUE(domain.GetDomainSuffix() == NULL);
    }
    {
        Domain domain("news.qq.com:123");
        EXPECT_TRUE(domain.GetDomainSuffix() == std::string(".com"));
    }
}

TEST(Domain, GetDomain)
{
    EXPECT_TRUE(Domain::Initialize("domainsuffix.txt", NULL, NULL));
    { Domain domain("219.238.187.126:9053"); ASSERT_TRUE(std::string(domain.GetDomain()) == std::string("219.238.187.126")); }
    { Domain domain("219.238.187.126:90531"); ASSERT_TRUE(std::string(domain.GetDomain()) == std::string("")); }
    {
        Domain domain("news.qq.com");
        ASSERT_EQ(std::string(domain.GetDomain()), std::string("qq.com"));
    }
    {
        Domain domain("news.Qq.cOM");
        ASSERT_EQ(std::string(domain.GetDomain()), std::string("qq.com"));
    }
    {
        Domain domain("abcd.info");
        ASSERT_EQ(std::string(domain.GetDomain()), std::string("abcd.info"));
    }
    {
        Domain domain("news.sina.com.cn");
        ASSERT_EQ(std::string(domain.GetDomain()), std::string("sina.com.cn"));
    }
    { Domain domain("news.djq"); ASSERT_TRUE(std::string(domain.GetDomain()) == std::string("")); }
    { Domain domain("127.0.0.1"); ASSERT_TRUE(std::string(domain.GetDomain()) == std::string("127.0.0.1")); }
    { Domain domain("127.0.0.1:80"); ASSERT_TRUE(std::string(domain.GetDomain()) == std::string("127.0.0.1")); }
    {
        Domain domain("219.238.187.126:9053");
        EXPECT_TRUE(domain.IsValidDomain());
    }
    { Domain domain("news.qq.com:80"); ASSERT_TRUE(std::string(domain.GetDomain()) == std::string("qq.com")); }
        
}

TEST(Domain, IsValidDomain)
{
    { Domain domain("news.qq.com:123"); ASSERT_EQ(domain.IsValidDomain(), true); }
    { Domain domain(".QQ.COM"); ASSERT_EQ(domain.IsValidDomain(), false); }
    { Domain domain("news.djq"); ASSERT_EQ(domain.IsValidDomain(), false); }
    { Domain domain("news.djq:123"); ASSERT_EQ(domain.IsValidDomain(), false); }
    { Domain domain("NEWS.QQ.COM"); ASSERT_EQ(domain.IsValidDomain(), true); }
    { Domain domain("news.qq.com"); ASSERT_EQ(domain.IsValidDomain(), true); }

    { Domain domain("172.0.0.1"); ASSERT_EQ(domain.IsValidDomain(), true); }
    { Domain domain("0.0.0.0"); ASSERT_EQ(domain.IsValidDomain(), true); }
    { Domain domain("172.0.0.1:65536"); ASSERT_EQ(domain.IsValidDomain(), false); }
    { Domain domain("172.0.0.1:65535"); ASSERT_EQ(domain.IsValidDomain(), true); }
    { Domain domain("10.11.1"); ASSERT_EQ(domain.IsValidDomain(), false); }
    { Domain domain("10.11.1.1.1"); ASSERT_EQ(domain.IsValidDomain(), false); }
    { Domain domain("10.11.1.256"); ASSERT_EQ(domain.IsValidDomain(), false); }

    // 连续的分隔符号
    { Domain domain("news..qq.com"); ASSERT_EQ(domain.IsValidDomain(), false); }
    { Domain domain("news.qq.com::100"); ASSERT_EQ(domain.IsValidDomain(), false); }

    // . 过多
    { Domain domain("1.2.3.4.5.com"); ASSERT_EQ(domain.IsValidDomain(), true); }
    { Domain domain("1.2.3.4.5.6.7.com"); ASSERT_EQ(domain.IsValidDomain(), false); }

    // 端口后面又出现了字母
    { Domain domain("www.qq.com:123a"); ASSERT_EQ(domain.IsValidDomain(), false); }
    { Domain domain("www.qq.com:123a1"); ASSERT_EQ(domain.IsValidDomain(), false); }


    { Domain domain(".com"); ASSERT_EQ(domain.IsValidDomain(), false); }
    { Domain domain("com"); ASSERT_EQ(domain.IsValidDomain(), false); }
    { Domain domain("255.255.255.255:65535"); EXPECT_TRUE(domain.IsValidDomain()); }
    { Domain domain("255.255.255.255:65536"); EXPECT_FALSE(domain.IsValidDomain()); }
    { Domain domain("�Ž�ǿ.com"); EXPECT_FALSE(domain.IsValidDomain()); }
}

TEST(Domain, GetScheSubdomain)
{
    EXPECT_TRUE(Domain::Initialize("domainsuffix.txt", "blogsuffix.txt", NULL));
    {
        Domain domain("news.qq.com:123");
        char* p = NULL;
        bool bRet = domain.GetScheSubdomain(p);
        ASSERT_EQ(bRet, false);
        ASSERT_EQ(std::string("news.qq.com:123"), std::string(p));
    }

    {
        Domain domain("news.0-6.com");
        char* p = NULL;
        bool bRet = domain.GetScheSubdomain(p);
        ASSERT_EQ(bRet, true);
        ASSERT_EQ(std::string("sosospider.0-6.com"), std::string(p));
    }
    {
        Domain domain("www.0-6.com");
        char* p = NULL;
        bool bRet = domain.GetScheSubdomain(p);
        ASSERT_EQ(bRet, false);
        ASSERT_EQ(std::string("www.0-6.com"), std::string(p));
    }
    {
        Domain domain("bj.1998.cn");
        char* p = NULL;
        bool bRet = domain.GetScheSubdomain(p);
        ASSERT_EQ(bRet, false);
        ASSERT_EQ(std::string("bj.1998.cn"), std::string(p));
    }
    {
        Domain domain("djq.1998.cn");
        char* p = NULL;
        bool bRet = domain.GetScheSubdomain(p);
        ASSERT_EQ(bRet, true);
        ASSERT_EQ(std::string("sosospider.1998.cn"), std::string(p));
    }
}

TEST(Domain, Performance)
{
    EXPECT_TRUE(Domain::Initialize("domainsuffix.txt", "blogsuffix.txt", NULL));
    ifstream file1("domain_test.txt");
    string line;

    vector<string> vec;
    while (getline(file1, line))
    {
        vec.push_back(line);
    }

    time_t begin = time(0);
    for (size_t i = 0; i < vec.size(); i++)
    {
        Domain domain(vec[i].c_str());
        domain.GetDomain();
    }
    time_t end = time(0);
    float speed = 0;
    if (end - begin != 0)
    {
        speed = vec.size() / (end - begin);
        cout << "load domain " << vec.size() << ", GetDomain speed: " << speed << endl;
    }

    begin = time(0);
    for (size_t i = 0; i < vec.size(); i++)
    {
        char* p = NULL;
        Domain domain(vec[i].c_str());
        domain.GetScheSubdomain(p);
    }
    end = time(0);
    if (end - begin != 0)
    {
        speed = vec.size() / (end - begin);
        cout << "load domain " << vec.size() << ", GetScheSubdomain speed: " << speed << endl;
    }
}

// XXX(phongchen): disable this test defaultly, network may be not unavailable
TEST(Domain, DISABLED_GetDataFromServer)
{
    EXPECT_TRUE(Domain::Initialize(SUFFIX_SERVER_IP, SUFFIX_SERVER_PORT, true, true));
    {
        Domain domain("news.0-6.com");
        char* p = NULL;
        bool bRet = domain.GetScheSubdomain(p);
        ASSERT_EQ(bRet, true);
        ASSERT_EQ(std::string("sosospider.0-6.com"), std::string(p));
    }
    {
        Domain domain("www.0-6.com");
        char* p = NULL;
        bool bRet = domain.GetScheSubdomain(p);
        ASSERT_EQ(bRet, false);
        ASSERT_EQ(std::string("www.0-6.com"), std::string(p));
    }
    {
        Domain domain("sosospider.0-6.com");
        char* p = NULL;
        bool bRet = domain.GetScheSubdomain(p);
        ASSERT_EQ(bRet, true);
        ASSERT_EQ(std::string("sosospider.0-6.com"), std::string(p));
    }
    {
        Domain domain("news.qq.com");
        char* p = NULL;
        bool bRet = domain.GetRelScheSubdomain(p);
        ASSERT_EQ(bRet, false);
        ASSERT_EQ(std::string("news.qq.com"), std::string(p));
    }
    {
        Domain domain("alexdu.blog.sohu.com");
        char* p = NULL;
        bool bRet = domain.GetRelScheSubdomain(p);
        ASSERT_EQ(bRet, true);
        ASSERT_EQ(std::string(".blog.sohu.com"), std::string(p));
    }
    {
        Domain domain("alexdu.sina.com.cn");
        char* p = NULL;
        bool bRet = domain.GetRelScheSubdomain(p);
        ASSERT_EQ(bRet, false);
        ASSERT_EQ(std::string("alexdu.sina.com.cn"), std::string(p));
    }
}

TEST(Domain, GetReverseSubDomain)
{
    char dest[1024];
    Domain::GetReverseSubDomain(dest, (char*)"news.qq.com");
    ASSERT_EQ(std::string("com.qq.news"), std::string(dest));
    
    Domain::GetReverseSubDomain(dest, (char*)"news.qq.com:80");
    ASSERT_EQ(std::string("com.qq.news:80"), std::string(dest));
    
    Domain::GetReverseSubDomain(dest, (char*)"news.qq.com:80:80");
    ASSERT_EQ(std::string("com.qq.news:80:80"), std::string(dest));
    
    Domain::GetReverseSubDomain(dest, (char*)"news.qq.com:90/1.html", 14);
    ASSERT_EQ(std::string("com.qq.news:90"), std::string(dest));
    
    Domain::GetReverseSubDomain(dest, (char*)"this.is.a.test");
    ASSERT_EQ(std::string("test.a.is.this"), std::string(dest));
}

TEST(Domain, Reload)
{
    EXPECT_TRUE(Domain::Initialize(SUFFIX_SERVER_IP, SUFFIX_SERVER_PORT, true, true));
    unsigned int reload_times = 10000;

    unsigned int begin = time(NULL);
    for (unsigned int i = 0; i < reload_times; i ++)
    {
        bool ret = Domain::Reload();
        ASSERT_TRUE(ret);
    }
    unsigned int end = time(NULL);
    cout << "avg reload speed: " << reload_times / (end - begin) << endl;
}

/*
TEST(domain, GetSubdomainID)
{
    {
        Domain domain("news.qq.com");
        unsigned long long sub_domain_id = domain.GetSubdomainID();
        EXPECT_EQ(sub_domain_id, 12993808137765405488ull);
    }
    {
        Domain domain("news.qq.abc");
        unsigned long long sub_domain_id = domain.GetSubdomainID();
        EXPECT_EQ(sub_domain_id, 0ull);
    }
}

TEST(domain, GetDomainID)
{
    {
        Domain domain("news.qq.com");
        unsigned long long sub_domain_id = domain.GetSubdomainID();
        EXPECT_EQ(sub_domain_id, 12993808137765405488ull);
    }
    {
        Domain domain("news.qq.abc");
        unsigned long long sub_domain_id = domain.GetSubdomainID();
        EXPECT_EQ(sub_domain_id, 0ull);
    }
}
*/
