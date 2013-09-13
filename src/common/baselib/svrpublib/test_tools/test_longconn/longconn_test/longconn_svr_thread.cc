#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test/head_files.h"
DECLARE_USING_LOG_LEVEL_NAMESPACE;

DECLARE_int32(to_port);
DECLARE_string(to_host);

CLongConnSvrThread::CLongConnSvrThread() {
    LOG(INFO) << "long conn. test, *server* side!";

    // get host,port
    const char* host = FLAGS_to_host.c_str();
    uint16_t    port = FLAGS_to_port;
    uint32_t    timeout = 2; // seconds


    if (!m_svr_obj.Init(host, port, timeout)) {
        LOG(ERROR) << "init fail...";
        return;
    }
}

CLongConnSvrThread::~CLongConnSvrThread() {
    m_svr_obj.Uninit();
    LOG(INFO) << "long connect server exit";
}

void CLongConnSvrThread::Routine() {
    m_svr_obj.m_longconn->RoutineLongConn(1000);

    return;
}
