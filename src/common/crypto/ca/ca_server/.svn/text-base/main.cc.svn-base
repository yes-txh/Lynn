// main.cc
#include "thirdparty/glog/logging.h"
#include "thirdparty/gflags/gflags.h"
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/crypto/ca/ca_server/my_ca_simple_http.h"

using namespace ca;

bool g_is_shut_down = false;
const uint32_t kTimeOut = 5;
const uint32_t kMaxBusinessThreadNum = 6;
const uint32_t kMaxFdLimit = 40000;
CSimpleHttpReceiveThread* g_http_receive_data_thread = NULL;
MySimpleHttpThread* g_http_business_thread = NULL;

void DealSigInt(int32_t sig) {
    VLOG(1)  << "catch a termination single,sig number " << sig;
    g_is_shut_down = true;
}

DECLARE_USING_LOG_LEVEL_NAMESPACE
//设置日志级别
void LogLevelSettings() {
#ifdef _DEBUG
    InitGoogleDefaultLogParam();
    FLAGS_minloglevel = 0;
    FLAGS_stderrthreshold = 0;
#else
    InitGoogleDefaultLogParam();
    FLAGS_minloglevel = 4;
    FLAGS_stderrthreshold = 4;
#endif
}

///////////////////////////////////////////////////////////////////////////
// 功能描述: main初始化
// 输入参数:
//           ip, 监听ip地址;
//           port，监听的端口
// 返回值:   true表示成功，false表示失败;
bool Init(const char* ip, const char* port) {
    // init role manager
    RoleManager::GetInstance();
    // init quota manager
    QuotaManager::GetInstance();
    
    uint16_t    http_port = ATOI(port);
    bool is_ok = true;
    // simple http receive data thread
    g_http_receive_data_thread = new CSimpleHttpReceiveThread;
    if (!g_http_receive_data_thread->Init(ip, http_port, kTimeOut)) {
        LOG(ERROR) << "start receive_data thread(Port:" 
                   << http_port << ") fail, maybe the port is in use.";
        return false;
    } else {
        VLOG(1) <<"start receive_data(Port " << http_port << ") ok.";
    }

    // simple http business threads
    g_http_business_thread = new MySimpleHttpThread[kMaxBusinessThreadNum];
    for (uint32_t u = 0; u < kMaxBusinessThreadNum; u++) {
        g_http_business_thread[u].Init();
        g_http_receive_data_thread->SetOutQueue(
            g_http_business_thread[u].GetInputQueueInterface());
        g_http_business_thread[u].SetInfo(ip, http_port);
        // start thread
        is_ok &= g_http_business_thread[u].StartThread();
    }    
    g_http_receive_data_thread->StartThread();

    return is_ok;
}

///////////////////////////////////////////////////////////////////////////
// 功能描述: main反初始化
// 输入参数:
//           无
// 返回值:   void
void UnInit() {
    // 释放simple http
    g_http_receive_data_thread->EndThread();
    g_http_receive_data_thread->Uninit();
    delete g_http_receive_data_thread;
    g_http_receive_data_thread = NULL;
    for (uint32_t u = 0; u < kMaxBusinessThreadNum; u++) {
        g_http_business_thread[u].EndThread();
    }
    delete []g_http_business_thread;
    g_http_business_thread = NULL;

    QuotaManager::FreeInstance();
    RoleManager::FreeInstance();
}

int32_t main(int32_t argc, char* argv[]) {
    LogLevelSettings();
    AutoBaseLib auto_baselib;

    //解析命令行参数
    google::AllowCommandLineReparsing();
    if ( static_cast<uint32_t>(1) != google::ParseCommandLineFlags(&argc, &argv, true)) {
        LOG(ERROR) << "google::ParseCommandLineFlags return false";
        return -1;
    }

    if (argc != 3) {
        LOG(ERROR) << "###usage:%s" << argv[0] << " <server ip> <port>";
        return -1;
    }

    // 处理Ctrl+C 消息,15 SIGTERM
    signal(SIGINT, &DealSigInt);
    signal(SIGTERM, &DealSigInt);

    CXSocketLibAutoManage auto_socket_lib_manage;

    // ignore signals
    IgnoreSig();

    // set core limit
    SetCoreLimit();

    SetFDLimit(kMaxFdLimit);

    xfs::base::SetModuleName("CA-Server");

    if(!Init(argv[1], argv[2])) {
        UnInit();
        LOG(ERROR) << "Init thread failed...";
        return -1;
    }

    // listen端口是否需要关闭
    while (!g_is_shut_down) {
        // 驱动线程运行
        XSleep(100);
    }

    UnInit();
    return 0;
}

