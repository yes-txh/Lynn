// LongConnTest.cpp : Defines the entry point for the console application.
//
#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test_svr/head_files.h"
using namespace xfs::base;
#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test_svr/test_obj.h"

bool InitSocketLib() {
    bool b =false;
#ifdef WIN32
    WSADATA wsaData= {0};
    WORD wVersionRequested = MAKEWORD( 2, 2 );
    int32_t nErr = WSAStartup(wVersionRequested, &wsaData );
    if ( nErr != 0 ) {
        b=FALSE;
    } else {
        if ( LOBYTE( wsaData.wVersion ) != 2 ||  HIBYTE( wsaData.wVersion ) != 2 ) {
            WSACleanup( );
            b=FALSE;
        } else {
            b=TRUE;
        }
    }
#else
    b = true;
#endif //_LINUX_OS_
    return b;
}

DEFINE_int32(to_port, 9900, "long connection server port");
DEFINE_string(to_host, "127.0.0.101", "connect to host,ip");

bool g_server_continue = true;

void DealSigInt(int sig) {
    LOG(WARNING) << "Catch a termination single,sig number:" << sig;
    g_server_continue = false;
}

int main(int argc, char* argv[]) {
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, false);

    // 处理Ctrl+C 消息,15 SIGTERM
    signal(SIGINT, &DealSigInt);
    signal(SIGTERM, &DealSigInt);

    LOG(INFO) << "long conn. test, *server* side!";
    LOG(INFO) << "usage: " << argv[0] << " listen host,port";

    // get host,port
    const char* host = FLAGS_to_host.c_str();
    uint16_t    port = FLAGS_to_port;
    uint32_t    timeout = 2; // seconds

    // init on WIN32
    InitSocketLib();
    AutoBaseLib abl;
    CBaseProtocolPack pack;
    pack.Init();

    CTestObj obj;
    if(!obj.Init(host,port,timeout)) {
        LOG(ERROR) << "init fail...";
        return -1;
    }

    while(g_server_continue) {
        obj.m_longconn->RoutineLongConn(1000);
        //XSleep(10);
    }
    obj.Uninit();
    pack.Uninit();

    return 0;
}
