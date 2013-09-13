// LongConnTest.cpp : Defines the entry point for the console application.
//
#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test/head_files.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;

DEFINE_int32(to_port, 9900, "long connection server port");
DEFINE_string(to_host, "127.0.0.101", "connect to host,ip");
// --method=default:start server and client
// --method=server:start server only
// --method=client:start client only
DEFINE_string(method, "default", "start server and client");
// --continue=true:circus test
// --continue=false:exit after 5s
DEFINE_bool(continue, false, "exit after 5s");

bool g_continue = true;
void DealSigInt(int sig) {
    LOG(WARNING) << "Catch a termination single,sig number:" << sig;
    g_continue = false;
}

int main(int argc, char* argv[])
{
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, false);

    SetCheckErrLevel(ERR);
    SetCheckProgramName(argv[0]);

    AutoBaseLib auto_base_lib;
    CXSocketLibAutoManage auto_lib_mgr;

    signal(SIGINT, &DealSigInt);
    signal(SIGTERM, &DealSigInt);

    // start server thread
    CLongConnSvrThread long_conn_svr_thread;
    if (FLAGS_method.compare("server") == 0 || FLAGS_method.compare("default") == 0) {
        if (!long_conn_svr_thread.StartThread()) {
            LOG(ERROR) << "long_conn_svr StartThread error";       
        }
    }

    // start client thread
    CLongConnClientThread long_conn_client_thread;
    if (FLAGS_method.compare("client") == 0 || FLAGS_method.compare("default") == 0) {
        if (!long_conn_client_thread.StartThread()) {
            LOG(ERROR) << "long_conn_client StartThread error";       
        }
    }

    if (FLAGS_continue) {
        // circus test
        while (g_continue) {
            XSleep(50);
        }
    } else {
        // exit after 5s
        uint32_t count = 5000 / 50;
        while (g_continue && count--) {
            XSleep(50);
        }
    }
    
    // end thread
    if (FLAGS_method.compare("server") == 0 || FLAGS_method.compare("default") == 0) {
        XSleep(5000);
        long_conn_svr_thread.EndThread();
    }
    if (FLAGS_method.compare("client") == 0 || FLAGS_method.compare("default") == 0) {
        long_conn_client_thread.EndThread();
    }
    
	return 0;
}

