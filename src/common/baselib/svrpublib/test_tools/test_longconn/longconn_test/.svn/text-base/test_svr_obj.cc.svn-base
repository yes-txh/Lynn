// TestObj.cpp: implementation of the CTestSvrObj class.
//
//////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test/head_files.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTestSvrObj::CTestSvrObj() {}

CTestSvrObj::~CTestSvrObj(){}

bool CTestSvrObj::InitSocketLib(){
    bool b = false;
#ifdef WIN32
    WSADATA wsaData = {0};    
    WORD wVersionRequested = MAKEWORD( 2, 2 );    
    int32_t nErr = WSAStartup(wVersionRequested, &wsaData );
    if ( nErr != 0 ) {        
        b = FALSE;
    } else {
        if ( LOBYTE( wsaData.wVersion ) != 2 ||  HIBYTE( wsaData.wVersion ) != 2 ) {
            WSACleanup( );
            b = FALSE; 
        } else {
            b = TRUE;
        }
    }
#else
    b = true;
#endif //_LINUX_OS_    
    return b;
}

bool CTestSvrObj::Init(const char* host, uint16_t port, uint32_t timeout) {
    // init on WIN32
    InitSocketLib();
    m_longconn = CreateLongConnObj();
    bool b = m_longconn?m_longconn->InitLongConn(this, 1024, host, port,
                                                 static_cast<float>(timeout)):false;
    
    m_pack.Init();
    m_unpack.Init();
    
    return b;
}

void CTestSvrObj::Uninit()
{
    if (m_longconn) {
        m_longconn->UninitLongConn();
        m_longconn->Release();
    }        
    m_longconn = 0; 
    
    m_pack.Uninit();
    m_unpack.Uninit();
}

uint32_t g_num_count = 0;
time_t g_t0;
void CTestSvrObj::OnTasksFinishedCallBack(LTasksGroup* task_group)
{
    if (g_num_count == 0)
        g_t0=time(0);
    g_num_count++;
    
    uint32_t ok_count = 0;
    for (uint32_t u = 0; u < task_group->m_valid_tasks; u++) {
        if (task_group->m_tasks[u].IsTaskFinished()) {
            ok_count++;
        }        
    }
    Xprintf("***total ok:%d***\r\n",ok_count);        
    
    if (g_num_count % 5000 == 0) {
        float favg = (static_cast<float>(g_num_count)) / (time(0)-g_t0);
        LOG(INFO) << "avg:" << favg;
    }
}

time_t g_s_t0 = 0;
unsigned int g_su_count = 0;

void CTestSvrObj::OnUserRequest(LongConnHandle session,
                                const unsigned char* data, uint32_t len,
                                bool& need_response){
#define TEST_SIMU_SERVER 1 
    
#ifdef TEST_SIMU_SERVER
    
    LTasksGroup task;

    m_unpack.AttachPackage(data, len);
    Xprintf_MemDebug((const char*)data, len);
    if (m_unpack.Unpack()) {
        // prepare response data
        // prepare data
        uint32_t seq = m_unpack.GetSeq();
        const char* msg = ":),response from server,test is ok!";
        m_pack.ResetContent();
        m_pack.SetSeq(seq); // set seq
        m_pack.SetServiceType(1028);//SS_SEARCH2SB_RSP
        m_pack.SetKey(123, msg);        
        
        // get response package
        unsigned char* pack_data = 0;
        uint32_t pack_len = 0;
        m_pack.GetPackage(&pack_data, &pack_len);
        
        // set send package
        task.m_tasks[0].SetSendData(pack_data, pack_len);
        // set session
        task.m_tasks[0].SetConnSession(session);
        // ? need response
        task.m_tasks[0].SetNeedResponse(0);
        
        // set valid tasks in group
        task.SetValidTasks(1);        
        
        // send data
        if (!m_longconn->SendData(&task)) {
            LOG(ERROR) << "send response data to client fail.uSeq=" << seq;
        }        
    } else {
        LOG(ERROR) << "***ERROR,Level 3***,"
               << "received user request,but unpack fail,pack len:" << len;
    }
    
#endif//    
    
    if (g_su_count == 0)
        g_s_t0 = time(0);
    g_su_count++;
    if (g_su_count % 10000 == 0) {
        float favg = static_cast<float>(g_su_count) / (time(0) - g_s_t0);
        LOG(INFO) << "server, response avg:" << favg << " times/s";
    }
}

void CTestSvrObj::OnClose(xfs::base::LongConnHandle) {
    LOG(INFO) << "server close";
}
