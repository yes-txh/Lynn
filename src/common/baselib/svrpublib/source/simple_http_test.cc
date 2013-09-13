//////////////////////////////////////////////////////////////////////////
// simple_http_test.cc
// @brief:     Test base class CBaseHttpProcThread
// @author:  fatliu@tencent
// @time:    2010-10-21
// @version: 1.0
//////////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

class MyBaseHttpProcThread:public CBaseHttpProcThread {
public:
    MyBaseHttpProcThread() {}
    ~MyBaseHttpProcThread() {}
    virtual bool OnUserHTTPRequest(const BufferV*   bufferv_recevied,
                                   CHttpBuff*       httpbuff_response) {
        const char* module = "my_simple_http_module";
        if (strcmp(reinterpret_cast<char*>(bufferv_recevied->buff), "/") == 0 ||
                strncmp(reinterpret_cast<char*>(bufferv_recevied->buff),
                        "/index.htm", 10) == 0) {
            httpbuff_response->SetAttr("my_simple_http", module, true);

            httpbuff_response->BeginGroup("test");
            httpbuff_response->AddKey("key1", "value1", "desc1");
            httpbuff_response->AddKey("key2", "value2", "desc2");
            httpbuff_response->AddKey("key3", "value3", "desc3");
            httpbuff_response->EndGroup("test");


            // 设置页面超链
            char link[1024];
            httpbuff_response->BeginGroup("MyTest");
            safe_snprintf(link,
                          sizeof(link)-1,
                          "http://%s/mytest.html",
                          GetListenHostPort());
            httpbuff_response->AddHref("test1.html", link, "mytest info");

            safe_snprintf(link,
                          sizeof(link)-1,
                          "http://%s/mytest.html",
                          GetListenHostPort());
            httpbuff_response->AddHref("test2.html", link, "mytest info");

            safe_snprintf(link,
                          sizeof(link)-1,
                          "http://%s/mytest.html",
                          GetListenHostPort());
            httpbuff_response->AddHref("test3.html", link, "mytest info");
            httpbuff_response->EndGroup("MyTest");

            return true;
        }

        if (strncmp(reinterpret_cast<char*>(bufferv_recevied->buff),
                    "/mytest.htm", 9) == 0) {
            // 设置页面标题
            httpbuff_response->SetAttr("meta search log", module);

            // 设置统计项 key - value 及描述信息
            httpbuff_response->BeginGroup("entry_mytest");
            httpbuff_response->AddKey("entry_mytest key1",
                                      "entry_mytest value1",
                                      "entry_mytest desc1");
            httpbuff_response->AddKey("entry_mytest key2",
                                      "entry_mytest value2",
                                      "entry_mytest desc2");
            httpbuff_response->AddKey("entry_mytest key3",
                                      "entry_mytest value3",
                                      "entry_mytest desc3");
            httpbuff_response->EndGroup("entry_mytest");

            return true;
        }

        return false;
    }
private:
};

#ifdef WIN32
int32_t TestSimpleHttp(int32_t argc, char** argv)
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

#ifdef WIN32
TEST(TestMyBaseHttpProcThread, MyBaseHttpProcThread) {
    const char* listen_host = "127.0.0.1";
    uint16_t listen_port = 51000;
    uint32_t timeout = 1;
    CSimpleHttpReceiveThread  recv_thread;
    if (!recv_thread.Init(listen_host, listen_port, timeout)) {
        VLOG(3) << "Try init receive data thread fail.";
        return;
    }

    // business threads
    MyBaseHttpProcThread    business_thread;
    business_thread.Init();
    recv_thread.SetOutQueue(
        business_thread.GetInputQueueInterface());
    bool is_ok = business_thread.StartThread();

    if (is_ok) {
        is_ok &= recv_thread.StartThread();
    }

    // http request, wait 5 minutes
    if (is_ok) {
        // XSleep(1000 * 60 * 5);
        XSleep(1000);
    }

    // stop all threads
    recv_thread.EndThread();
    recv_thread.Uninit();
    business_thread.EndThread();
}
#endif
