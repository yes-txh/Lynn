//////////////////////////////////////////////////////////////////////////
// parser_cgi_parameter_test.cc
// @brief:     Test class CParserCGIParameter & CParserKeyVal
// @author:  fatliu@tencent
// @time:     2010-09-29
// @version: 1.0
//////////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/general_head.h"
#include "common/baselib/svrpublib/general_type_def.h"
#include "common/baselib/svrpublib/thread_mutex.h"
#include "common/baselib/svrpublib/general_util.h"
#include "common/baselib/svrpublib/log.h"
#include "common/baselib/svrpublib/parser_cgi_parameter.h"
#include "common/baselib/svrpublib/generate_cgi_parameter.h"
#include "common/baselib/svrpublib/url_codec.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

#ifdef WIN32
int32_t TestParserGgiParameter(int32_t argc, char** argv)
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

TEST(TestParserCGIParameter, EnvironmentString) {
    CParserCGIParameter parser_cgi;
    char* str_buff =
        const_cast<char*>("f=8&wd=%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4&"
                          "test=a1.b2.c3");

    const char* str_val = NULL;
    uint32_t uint32_val = 0;
    int32_t int32_val = 0;

    // case1: attach environment string
    uint32_t ok = parser_cgi.AttachEnvironmentString(str_buff);
    CHECK_EQ(1, static_cast<int32_t>(ok));
    // get num
    uint32_t num = parser_cgi.GetParameterCount();
    CHECK_EQ(3, static_cast<int32_t>(num));

    // get parameter
    ok = parser_cgi.GetParameter("wd", &str_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(0, strncmp(str_val, "≤‚ ‘≤‚ ‘≤‚≤‚ ‘", strlen("≤‚ ‘≤‚ ‘≤‚≤‚ ‘")));
    ok = parser_cgi.GetParameter("f", &uint32_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(uint32_val, static_cast<uint32_t>(8));
    ok = parser_cgi.GetParameter("f", &int32_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(int32_val, (int32_t)(8));

    // get parameter name
    const char* str_name = parser_cgi.GetParameterName(0);
    CHECK_EQ(0, strncmp(str_name, "f", strlen("f")));

    // get parameter val
    const char* str_val1 = parser_cgi.GetParameterVal(1);
    CHECK_EQ(0, strncmp(str_val1, "≤‚ ‘≤‚ ‘≤‚≤‚ ‘", strlen("≤‚ ‘≤‚ ‘≤‚≤‚ ‘")));

    // default post method
    // set get method
    // undo...how to set get method
    // str_buff = "REQUEST_METHOD=GET"
    //            "&f=8&wd=%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4";
    // bOK = parser_cgi.ParserEnvironmentString(CParserCGIParameter::REQ_GET);
    // CHECK_EQ(1, bOK); //
    // bOK = parser_cgi.GetParameter("wd", &str_val);
    // CHECK_EQ(1, bOK);
    // CHECK_EQ(0, strncmp(str_val,
    //                  "%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4",
    //                  strlen("%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4")));
}

TEST(TestParserCGIParameter, Cookie) {
    // case2: cookie
    CParserCGIParameter parser_cgi;
    char* str_buff = const_cast<char*>("f=8;wd=%B2%E2%CA%D4%B2%E2%C"\
                                       "A%D4%B2%E2%B2%E2%CA%D4;test=a1.b2.c3");
    // attach cookie
    int32_t ok = parser_cgi.AttachCookie(str_buff);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    // get num
    uint32_t num = parser_cgi.GetCookieParameterCount();
    EXPECT_EQ(static_cast<uint32_t>(3), num);

    // get cookie parameter
    const char* str_val = NULL;
    uint32_t uint32_val = 0;
    int32_t int32_val = 0;
    ok = parser_cgi.GetCookieParameter("wd", &str_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(0, strncmp(str_val,
                        "%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4",
                        strlen("%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4")));

    ok = parser_cgi.GetCookieParameter("f", &uint32_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(uint32_val, static_cast<uint32_t>(8));
    ok = parser_cgi.GetCookieParameter("f", &int32_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(int32_val, static_cast<int32_t>(8));
}

TEST(TestParserKeyVal, UnicodeWithChinese) {
    CParserKeyVal parser_key_val;

    // normal
    // case1:unicode with chinese
    char* str_buff = const_cast<char*>(
                         "f=8&wd=%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4");

    uint32_t len = static_cast<uint32_t>(strlen(str_buff));
    // parser string
    uint32_t ok = parser_key_val.ParserString(str_buff, len, 1, '&', '=');
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    // get valid item count
    uint32_t num = parser_key_val.GetValidItemCount();
    CHECK_EQ(static_cast<uint32_t>(2), num);

    const char* str_val = NULL;
    const char* key_name0 = parser_key_val.GetKeyName(0);
    CHECK_EQ(0, strncmp(key_name0, "f", strlen("f")));
    const char* key_name1 = parser_key_val.GetKeyName(1);
    CHECK_EQ(0, strncmp(key_name1, "wd", strlen("wd")));

    ok = parser_key_val.GetValue("wd", &str_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(0, strncmp(str_val, "≤‚ ‘≤‚ ‘≤‚≤‚ ‘", strlen("≤‚ ‘≤‚ ‘≤‚≤‚ ‘")));
}

TEST(TestParserKeyVal, Normal) {
    CParserKeyVal parser_key_val;

    // case2
    char* str_buff = const_cast<char*>(
                         "TYPE=single&PROJID=380&SERVICEID=1000&SERVICEIP=172.16.81.31"
                     );

    uint32_t len = static_cast<uint32_t>(strlen(str_buff));
    int32_t ok = parser_key_val.ParserString(str_buff, len, 1,  '&', '=');
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    int32_t num = parser_key_val.GetValidItemCount();
    CHECK_EQ(static_cast<uint32_t>(4), num);

    // get key name
    const char* key_name0 = parser_key_val.GetKeyName(0);
    CHECK_EQ(0, strncmp(key_name0, "TYPE", strlen("TYPE")));
    const char* key_name1 = parser_key_val.GetKeyName(3);
    CHECK_EQ(0, strncmp(key_name1, "SERVICEIP", strlen("SERVICEIP")));

    // get value
    const char* str_val = NULL;
    ok = parser_key_val.GetValue("TYPE", &str_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(0, strncmp(str_val, "single", strlen("single")));
    uint32_t uint32_val = 0;
    ok = parser_key_val.GetValue("SERVICEID", &uint32_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(uint32_val, static_cast<uint32_t>(1000));
    uint16_t uint16_val = 0;
    ok = parser_key_val.GetValue("SERVICEID", &uint16_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(uint32_val, (uint16_t)(1000));
    int32_t int32_val = 0;
    ok = parser_key_val.GetValue("SERVICEID", &int32_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(uint32_val, static_cast<uint32_t>(1000));
    str_val = parser_key_val.GetValue("SERVICEIP");
    CHECK_EQ(0, strncmp(str_val, "172.16.81.31", strlen("172.16.81.31")));

    // get value string
    const char* str_val1 = parser_key_val.GetValString(0);
    CHECK_EQ(0, strncmp(str_val1, "single", strlen("single")));
}

TEST(TestParserKeyVal, NotUnicode) {
    CParserKeyVal parser_key_val;

    // case3: not unicode
    char* str_buff = const_cast<char*>(
                         "f=8&wd=%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4"
                     );

    const char* str_val = NULL;
    uint32_t len = static_cast<uint32_t>(strlen(str_buff));
    int32_t ok = parser_key_val.ParserString(str_buff, len, 0, '&', '=');
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    int32_t num = parser_key_val.GetValidItemCount();
    CHECK_EQ(static_cast<uint32_t>(2), num);
    ok = parser_key_val.GetValue("wd", &str_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_NE(0, strncmp(str_val, "≤‚ ‘≤‚ ‘≤‚≤‚ ‘", strlen("≤‚ ‘≤‚ ‘≤‚≤‚ ‘")));
    ok = parser_key_val.GetValue("wd", &str_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(0, strncmp(str_val,
                        "%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4",
                        strlen("%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4")));
}

TEST(TestGenerateUrl, normal) {
    GenerateCGIParameter generate_cgi;
    CParserCGIParameter parser_cgi;
    CParserKeyVal parser_key_val;
    std::string str_domain = std::string("http://www.xfs.soso.com");
    std::string str_cgi = std::string("proxy.cgi");
    generate_cgi.SetDomain(str_domain);
    generate_cgi.SetCgi(str_cgi);
    std::string key = std::string("realserver");
    std::string value = std::string("http://172.26.1.184:8080/a.cgi?cleanpwd=xfs");
    generate_cgi.AppendParameter(key, value);
    key = std::string("home");
    value = std::string("myhome");
    generate_cgi.AppendParameter(key, value);
    std::string url = generate_cgi.GetGeneratedUrl();

    std::string parameter_content = parser_cgi.GetParmeterContent(url);
    parser_key_val.ParserString(parameter_content.c_str(),
                                static_cast<uint32_t>(parameter_content.size()));
    const char* str_val = parser_key_val.GetValue("realserver", true);
    CHECK_EQ(0, strncmp(str_val,
                        "http://172.26.1.184:8080/a.cgi?cleanpwd=xfs",
                        strlen("http://172.26.1.184:8080/a.cgi?cleanpwd=xfs")));
    str_val = parser_key_val.GetValue("home", true);
    CHECK_EQ(0, strncmp(str_val, "myhome", strlen("myhome")));

    GenerateCGIParameter generate_cgi2;
    generate_cgi2.SetDomain(str_domain);
    generate_cgi2.SetCgi(str_cgi);
    key = std::string("realserver");
    value = std::string("http://172.26.1.184:8080/a.cgi?cleanpwd%3Dxfs");
    generate_cgi2.AppendParameter(key, value);
    key = std::string("home");
    value = std::string("myhome");
    generate_cgi2.AppendParameter(key, value);

    url = generate_cgi2.GetGeneratedUrl();

    parameter_content = parser_cgi.GetParmeterContent(url);
    parser_key_val.ParserString(parameter_content.c_str(),
                                static_cast<uint32_t>(parameter_content.size()));
    str_val = parser_key_val.GetValue("realserver", true);
    CHECK_EQ(0, strncmp(str_val,
        "http://172.26.1.184:8080/a.cgi?cleanpwd%3Dxfs",
        strlen("http://172.26.1.184:8080/a.cgi?cleanpwd%3Dxfs"))) << str_val;
    str_val = parser_key_val.GetValue("home", true);
    CHECK_EQ(0, strncmp(str_val, "myhome", strlen("myhome")));
}

TEST(TestParserCGIParameter, EnvironmentStringNotDecode) {
    CParserCGIParameter parser_cgi;
    const char* str_buff = "f=8&wd=%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4";

    const char* str_val = NULL;
    std::string str_output;
    uint32_t uint32_val = 0;
    int32_t int32_val = 0;

    // case1: attach environment string
    uint32_t ok = parser_cgi.AttachEnvironmentString(str_buff, false);
    CHECK_EQ(1, static_cast<int32_t>(ok));
    // get num
    uint32_t num = parser_cgi.GetParameterCount();
    CHECK_EQ(2, static_cast<int32_t>(num));

    // get parameter
    ok = parser_cgi.GetParameter("wd", &str_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(0, strncmp(str_val, "%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4", 
                        strlen("%B2%E2%CA%D4%B2%E2%CA%D4%B2%E2%B2%E2%CA%D4")));
    ok = parser_cgi.GetParameter("f", &uint32_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(uint32_val, static_cast<uint32_t>(8));
    ok = parser_cgi.GetParameter("f", &int32_val);
    CHECK_EQ(static_cast<uint32_t>(1), ok);
    CHECK_EQ(int32_val, (int32_t)(8));

    // get parameter name
    const char* str_name = parser_cgi.GetParameterName(0);
    CHECK_EQ(0, strncmp(str_name, "f", strlen("f")));

    // get parameter val
    std::string str_val1 = parser_cgi.GetParameterVal(1);
    CHECK_EQ(true, UrlCodec::Decode(str_val1, &str_output));
    CHECK_EQ(0, strncmp(str_output.c_str(), "≤‚ ‘≤‚ ‘≤‚≤‚ ‘", strlen("≤‚ ‘≤‚ ‘≤‚≤‚ ‘")));
}
