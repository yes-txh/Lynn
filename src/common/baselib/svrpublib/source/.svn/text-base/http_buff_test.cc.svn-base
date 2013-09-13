//////////////////////////////////////////////////////////////////////////
// http_buff_test.cc
// @brief:     Test class CHttpBuff
// @author:  fatliu@tencent
// @time:    2010-10-3
// @version: 1.0
//////////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/general_head.h"
#include "common/baselib/svrpublib/general_type_def.h"
#include "common/baselib/svrpublib/thread_mutex.h"
#include "common/baselib/svrpublib/general_util.h"

#include "common/baselib/svrpublib/log.h"
#include "common/baselib/svrpublib/lite_mempool.h"
#include "common/baselib/svrpublib/general_sock.h"

#include "common/baselib/svrpublib/parser_cgi_parameter.h"
#include "common/baselib/svrpublib/utf8.h"
#include "common/baselib/svrpublib/http_buff.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE
using namespace xfs::base;

#ifdef WIN32
int32_t TestHttpBuff(int32_t argc, char** argv)
#else
int32_t main(int32_t argc, char** argv)
#endif
{
#ifndef WIN32
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);

    AutoBaseLib auto_baselib;
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
#else
    return 0;
#endif
}

// @brief:     Test method SetContentType
TEST(CHttpBuff, SetContentType) {
    CHttpBuff http_buff;
    bool b = false;

    http_buff.SetContentType(ENUM_HTTP_HTML);
    b = http_buff.IsXML();
    CHECK(!b);
    http_buff.SetContentType(ENUM_HTTP_XML);
    b = http_buff.IsXML();
    CHECK(b);
}

// @brief:     Test method XmlBeginEndGroup
TEST(CHttpBuff, XmlBeginEndGroup) {
    CHttpBuff http_buff;

    http_buff.ResetHttpContent();
    http_buff.SetContentType(ENUM_HTTP_XML);

    const char* name = "fat test";
    http_buff.BeginGroup(name);
    http_buff.EndGroup(name);

    const char* out_content = NULL;
    uint32_t out_len = 0;
    http_buff.GetBody(&out_content, &out_len);
#ifdef WIN32
    char* in_content = const_cast<char*>("<fat test>\r\n"
                                         "</fat test>\r\n"
                                         "<Link>\r\n");
#else
    char* in_content = const_cast<char*>("<fat test>\r\n"
                                         "</fat test>\r\n"
                                         "<Link>\r\n");
#endif
    const char* p = strstr((const char*)out_content, (const char*)in_content);
    CHECK(p);
}

// @brief:     Test method XmlAddkey
TEST(CHttpBuff, XmlAddkey) {
    CHttpBuff http_buff;

    http_buff.ResetHttpContent();
    http_buff.SetContentType(ENUM_HTTP_XML);

    http_buff.AddKey("key1", "value1", "desc1");
    http_buff.AddKey("key2", static_cast<float>(2.0), "desc2");
    http_buff.AddKey("key3", static_cast<uint32_t>(3), "desc3");
    http_buff.AddKey("key4", static_cast<uint64_t>(4), "desc4");
    http_buff.AddKey("key5", static_cast<uint16_t>(5), "desc5");
    const char* in_content = "    <key1 desc='desc1'>value1</key1>\r\n"
                             "    <key2 desc='desc2'>2.00</key2>\r\n"
                             "    <key3 desc='desc3'>3</key3>\r\n"
                             "    <key4 desc='desc4'>4</key4>\r\n"
                             "    <key5 desc='desc5'>5</key5>\r\n";
    uint32_t in_len = static_cast<uint32_t>(strlen(in_content));

    const char* out_content = NULL;
    uint32_t out_len = 0;
    http_buff.GetBody(&out_content, &out_len);
    CHECK_EQ(0, strncmp((const char*)out_content, in_content, in_len));
}

// @brief:     Test method XmlAddHref
TEST(CHttpBuff, XmlAddHref) {
    CHttpBuff http_buff;

    http_buff.ResetHttpContent();
    http_buff.SetContentType(ENUM_HTTP_XML);
    http_buff.AddHref("title",
                      "http://www.test.com/href?key1=value1&key2=value2",
                      "desc");

    const  char* out_content = NULL;
    uint32_t out_len = 0;
    http_buff.GetBody(&out_content, &out_len);

    const char* p = strstr((const char*)out_content,
                           "<Title>title</Title>\r\n");
    CHECK(p);

    p = strstr((const char*)out_content,
               "    <Desc>desc</Desc>\r\n");
    CHECK(p);

    p = strstr((const char*)out_content,
               "<Url>http://www.test.com/href?");
    CHECK(p);

    p = strstr((const char*)out_content,
               "key1=value1&amp;key2=value2");
    CHECK(p);
}

// @brief:     Test method SetProxy
TEST(CHttpBuff, SetProxy) {
    CHttpBuff http_buff;

    http_buff.ResetHttpContent();

    http_buff.SetProxy("http:\\www.proxy.com");
    http_buff.AddHref("title",
                      "http://www.test.com/href?key1=value1&key2=value2",
                      "desc");
    const char* out_content = NULL;
    uint32_t out_len = 0;
    http_buff.GetBody(&out_content, &out_len);

    const char* p = strstr((const char*)out_content,
                           "http:\\www.proxy.com?realserver="
                           "http://www.test.com/href&");
    CHECK(p);
}

// @brief:     Test method SetHome
TEST(CHttpBuff, SetHome) {
    CHttpBuff http_buff;

    http_buff.ResetHttpContent();
    http_buff.SetContentType(ENUM_HTTP_XML);
    http_buff.SetHome("HomeUrl");

    const char* out_content = NULL;
    uint32_t out_len = 0;
    http_buff.GetBody(&out_content, &out_len);

    const char* p = strstr((const char*)out_content, "<Title>XFS</Title>");
    CHECK(p);
}

// @brief:     Test method SetHostPort
TEST(CHttpBuff, SetHostPort) {
    CHttpBuff http_buff;

    http_buff.ResetHttpContent();
    http_buff.SetContentType(ENUM_HTTP_XML);
    http_buff.SetHostPort("2010");

    const char* out_content = NULL;
    uint32_t out_len = 0;
    http_buff.GetBody(&out_content, &out_len);

    const char* p = strstr((const char*)out_content,
                           "http://2010/index.html");
    CHECK(p);
}

// @brief:     Test method FormatXmlString
TEST(CHttpBuff, FormatXmlString) {
    CHttpBuff http_buff;

    http_buff.ResetHttpContent();
    http_buff.SetContentType(ENUM_HTTP_XML);

    const char* buff_url = "abc\"abc\'abc&abc>abc<abc\'abc\"abc";
    const char* in_strbuff =
        "abc&quot;abc&apos;abc&amp;abc&gt;abc&lt;abc&apos;abc&quot;abc";
    uint32_t in_strlen = (uint32_t)strlen(in_strbuff);
    CStrBuff out_strbuff;
    http_buff.FormatXmlString(buff_url,
                              (int32_t)strlen(buff_url),
                              out_strbuff);
    CHECK_EQ(out_strbuff.GetValidLen(), in_strlen);
    CHECK_EQ(0, strncmp(out_strbuff.GetString(),
                        in_strbuff,
                        in_strlen));
}

// @brief:     Test method HtmlAddkey
TEST(CHttpBuff, HtmlAddkey) {
    // 不确认包体内容，暂时屏蔽
    return;
    CHttpBuff http_buff;

    http_buff.ResetHttpContent();
    http_buff.SetContentType(ENUM_HTTP_HTML);

    http_buff.AddKey("key", "value", "desc");

    const char* in_content = "key";
    uint32_t in_len = static_cast<uint32_t>(strlen(in_content));

    const char* out_content = NULL;
    uint32_t out_len = 0;
    http_buff.GetBody(&out_content, &out_len);
    CHECK_EQ(0, strncmp((const char*)out_content, in_content, in_len));
}

// @brief:     Test method HtmlAddHref
TEST(CHttpBuff, HtmlAddHref) {
    CHttpBuff http_buff;

    http_buff.ResetHttpContent();
    http_buff.SetContentType(ENUM_HTTP_HTML);
    http_buff.AddHref("title",
                      "http://www.test.com/href?key1=value1&key2=value2",
                      "desc");

    const char* out_content = NULL;
    uint32_t out_len = 0;
    http_buff.GetBody(&out_content, &out_len);

    const char* p = strstr(out_content, "key1=value1&key2=value2");
    CHECK(p);
}

// @brief:     Test method SetAttr
TEST(CHttpBuff, SetAttr) {
    CHttpBuff http_buff;

    http_buff.ResetHttpContent();
    http_buff.SetAttr("title", "module", true);

    const char* out_content = NULL;
    uint32_t out_len = 0;
    http_buff.GetBody(&out_content, &out_len);
    const char* p = NULL;

#ifdef WIN32
    const char* ktext = "系统信息 CPU MEM DISK NET 等";
    char sz_buff[256] = {0};
    bool b = ConvertAnsiToUTF8(ktext,
                               (int32_t)strlen(ktext),
                               sz_buff,
                               (int32_t)sizeof(sz_buff),
                               reinterpret_cast<int32_t*>(&out_len));
    CHECK(b);
    http_buff.GetHead(&out_content, &out_len);
    p = strstr(out_content, sz_buff);
    CHECK(!p);
#endif

    http_buff.GetHead(&out_content, &out_len);
    p = strstr(out_content,
               "<title>title</title>");
    CHECK(p);
}
