// main.cc

#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/baselib/svrpublib/test_tools/test_http_service/my_simple_http.h"

bool g_is_shut_down = false;

void DealSigInt(int32_t sig)
{
    LOG(WARNING) << "catch a termination single,sig number: " << sig;
    g_is_shut_down = true;
}

int32_t main(int32_t argc, char* argv[])
{
    InitGoogleDefaultLogParam(argv[0]);
    google::ParseCommandLineFlags(&argc, &argv, true);

    AutoBaseLib auto_baselib;

    // 处理Ctrl+C 消息,15 SIGTERM
    signal(SIGINT, &DealSigInt);
    signal(SIGTERM, &DealSigInt);

    CXSocketLibAutoManage auto_socket_lib_manage;

    // ignore signals
    IgnoreSig();

    // set core limit
    SetCoreLimit();

    xfs::base::SetModuleName("Test HTTP Service");

    if (argc != 3) {
        LOG(INFO) << "###usage:" << argv[0] << " <server ip1> <port>";
        return -1;
    }

    uint16_t    http_port = ATOI(argv[2]);
    const char* host = argv[1];

    // init daemon
#ifndef WIN32
    // daemon(1, 0);
#endif

    // simple http receive data thread
    CSimpleHttpReceiveThread  http_receive_data_thread;
    if (!http_receive_data_thread.Init(host, http_port, 5)) {
        LOG(ERROR) << " start http_receive_data_thread Port:" << http_port
                   << " fail, maybe the port is in use.";
    } else {
        LOG(INFO) << "start simple http thread(" << host << " : " << http_port << ") ok.";
    }

    // simple http business threads
    CMySimpleHttpThread http_business_thread[2];
    
    http_business_thread[0].Init();
    http_business_thread[1].Init();
	http_receive_data_thread.SetOutQueue(http_business_thread[0].GetInputQueueInterface());
    http_receive_data_thread.SetOutQueue(http_business_thread[1].GetInputQueueInterface());
    http_business_thread[0].SetInfo(host, http_port);
    http_business_thread[1].SetInfo(host, http_port);
    // start thread
    bool is_ok = http_business_thread[0].StartThread();
    is_ok = http_business_thread[1].StartThread();
    if (is_ok)
        is_ok &= http_receive_data_thread.StartThread();


    // listen端口是否需要关闭
    while (!g_is_shut_down) {
        // 驱动线程运行
        XSleep(100);
    }

    // 释放simple http
    http_receive_data_thread.EndThread();
    http_business_thread[0].EndThread();
    http_business_thread[1].EndThread();

    return 0;
}
