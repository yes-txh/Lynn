//////////////////////////////////////////////////////////////////////////
// http_content_test.cc
// @brief:   对分析从cgi返回的响应数据的类CHttpContent和ultiPartDataParser
//           的测试
// @author:  fatliu@tencent
// @time:    2010-09-08
// @version: 1.0
//////////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/baselib/svrpublib/general_head.h"
#include "common/baselib/svrpublib/general_type_def.h"
#include "common/baselib/svrpublib/thread_mutex.h"
#include "common/baselib/svrpublib/general_util.h"
#include "common/baselib/svrpublib/log.h"
#include "common/baselib/svrpublib/check.h"
#include "common/baselib/svrpublib/lite_mempool.h"
#include "common/baselib/svrpublib/general_sock.h"

#include "xfs/scheduler/scheduler_master_v2/http_content.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;

#ifdef WIN32
///////////////////////////////////////////////////////////////////////////
// 功能描述: 测试HttpContent的主函数
// 输入参数:
//           无
// 返回值:   0表示成功，-1表示失败;
int TestHttpContent(int argc, char** argv);
#endif

///////////////////////////////////////////////////////////////////////////
// 功能描述: 测试GetValue是否能正确获取key:value
// 输入参数:
//           无
// 返回值:   无
void TestGetValue(CHttpContent* obj_http_content);

///////////////////////////////////////////////////////////////////////////
// 功能描述: 测试获取transfer-encoding类型
// 输入参数:
//           无
// 返回值:   无
void TestGetTransferEncoding(CHttpContent* obj_http_content);

///////////////////////////////////////////////////////////////////////////
// 功能描述: 测试获取transfer-Content类型
// 输入参数:
//           无
// 返回值:   无
void TestGetContentEncoding(CHttpContent* obj_http_content);

///////////////////////////////////////////////////////////////////////////
// 功能描述: 测试解析字符区域中的内容
// 输入参数:
//           无
// 返回值:   无
void TestParserFromBuff(CHttpContent* obj_http_content);

#ifdef WIN32
int TestHttpContent(int argc, char** argv)
#else // linux
int main(int argc, char** argv)
#endif
{
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, false);

    AutoBaseLib auto_baselib;
    SetCheckErrLevel(ERR);
    SetCheckProgramName(argv[0]);

    CHttpContent* obj_http_content = new CHttpContent();
    TestGetValue(obj_http_content);
    TestGetTransferEncoding(obj_http_content);
    TestGetContentEncoding(obj_http_content);
    TestParserFromBuff(obj_http_content);

    delete obj_http_content;
    return 0;
}

void TestGetValue(CHttpContent* obj_http_content) {
    const char* ptr_head = "HTTP/1.1 200 OK\n"
                           "Keep-Alive: timeout=5, max=100\n"
                           "Date: Wed, 08 Sep 2010 01:52:53 GMT\n"
                           "Server: Apache/2.2.9 (Unix)\n"
                           "Connection: Keep-Alive\n"
                           "Transfer-Encoding: chunked\n"
                           "Content-Type: text/html";

    const uint32_t head_len = static_cast<uint32_t>(strlen(ptr_head));
    char* ptr_val = 0;
    uint32_t val_len = 0;
    const char* ptr_key_name = "Keep-Alive";
    obj_http_content->GetValue((unsigned char*)ptr_head, head_len, (unsigned char*)ptr_key_name, 
                               (unsigned char**)(&ptr_val), &val_len);

    const char* ptr_key_val = "timeout=5, max=100";
    const uint32_t key_val_len = static_cast<uint32_t>(strlen(ptr_key_val));
    CHECK_EQ(val_len, key_val_len);
    CHECK_EQ(0, memcmp(ptr_val, ptr_key_val, key_val_len));
}

void TestGetTransferEncoding(CHttpContent* obj_http_content) {
    const char* ptr_head = "HTTP/1.1 200 OK\n"
                           "Keep-Alive: timeout=5, max=100\n"
                           "Date: Wed, 08 Sep 2010 01:52:53 GMT\n"
                           "Server: Apache/2.2.9 (Unix)\n"
                           "Connection: Keep-Alive\n"
                           "Transfer-Encoding: chunked\n"
                           "Content-Type: text/html";
    const uint32_t head_len = static_cast<uint32_t>(strlen(ptr_head));
    char* ptr_type = 0;
    uint32_t type_len = 0;
    obj_http_content->GetTransferEncoding((unsigned char*)ptr_head, head_len, (unsigned char**)&ptr_type, &type_len);

    CHECK_EQ(type_len, static_cast<uint32_t>(strlen("chunked")));
    CHECK_EQ(0, memcmp(ptr_type, "chunked", strlen("chunked")));
}

void TestGetContentEncoding(CHttpContent* obj_http_content) {
    const char* ptr_head = "HTTP/1.1 200 OK\n"
                           "Keep-Alive: timeout=5, max=100\n"
                           "Date: Wed, 08 Sep 2010 01:52:53 GMT\n"
                           "Server: Apache/2.2.9 (Unix)\n"
                           "Content-Encoding: gzip\n"
                           "Connection: Keep-Alive\n"
                           "Transfer-Encoding: chunked\n"
                           "Content-Type: text/html";
    const uint32_t head_len = static_cast<uint32_t>(strlen(ptr_head));
    char* ptr_type = 0;
    uint32_t type_len = 0;
    obj_http_content->GetContentEncoding((unsigned char*)ptr_head, head_len, (unsigned char**)&ptr_type, &type_len);

    CHECK_EQ(type_len, static_cast<uint32_t>(strlen("gzip")));
    CHECK_EQ(0, memcmp(ptr_type, "gzip", strlen("gzip")));
}

void TestParserFromBuff(CHttpContent* obj_http_content) {
    // chuncked
    // 长度后只有一个/n会出问题
    char* ptr_input_buff = const_cast<char*>("HTTP/1.1 200 OK\n"
                           "Keep-Alive: timeout=5, max=100\n"
                           "Date: Wed, 08 Sep 2010 01:52:53 GMT\n"
                           "Server: Apache/2.2.9 (Unix)\n"
                           "Connection: Keep-Alive\n"
                           "Transfer-Encoding: chunked\n"
                           "Content-Type: text/html\n\n\n"
                           "D\n\n"
                           "<html><head>\n"
                           "10A\n\n"
                           "<meta http-equiv='Content-Language' content='zh-cn'>"
                           "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
                           "<title>fat test ParserFromBuff</title></head>"
                           "<body><br><br>welcome~~~<br>"
                           "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa<br>"
                           "bbbbbbbbbbbb<br>"
                           "</body></html>\n0\n");
    const uint32_t buff_len = static_cast<uint32_t>(strlen(ptr_input_buff));
    unsigned char* ptr_pure_body = 0;
    uint32_t body_len = 0;
    obj_http_content->ParserFromBuff(ptr_input_buff, buff_len, &ptr_pure_body, &body_len);
    CHECK_EQ(279, static_cast<int32_t>(body_len));

    FILE* fp = fopen("http_content_test.html", "wb");
    fwrite(ptr_pure_body, body_len, 1, fp);
    fclose(fp);

    // 不为chuncked,存在Content-Length
    char* ptr_input_buff2 = const_cast<char*>("HTTP/1.1 200 OK\n"
                            "Keep-Alive: timeout=5, max=100\n"
                            "Date: Wed, 08 Sep 2010 01:52:53 GMT\n"
                            "Server: Apache/2.2.9 (Unix)\n"
                            "Connection: Keep-Alive\n"
                            "Content-Length: 279"
                            "Content-Type: text/html\n\n\n\n"
                            "<html><head>\n"
                            "<meta http-equiv='Content-Language' content='zh-cn'>"
                            "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
                            "<title>fat test ParserFromBuff</title></head>"
                            "<body><br><br>welcome~~~<br>"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa<br>"
                            "bbbbbbbbbbbb<br>"
                            "</body></html>\n0\n");

    const uint32_t buff_len2 = static_cast<uint32_t>(strlen(ptr_input_buff2));
    unsigned char* ptr_pure_body2 = 0;
    uint32_t body_len2 = 0;
    obj_http_content->ParserFromBuff(ptr_input_buff2, buff_len2, &ptr_pure_body2, &body_len2);
    CHECK_EQ(279, static_cast<int32_t>(body_len2));
}

