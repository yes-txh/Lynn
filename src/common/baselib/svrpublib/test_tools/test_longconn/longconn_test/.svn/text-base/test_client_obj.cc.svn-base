// TestObj.cpp: implementation of the CTestClientObj class.
//
//////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test/head_files.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTestClientObj::CTestClientObj()
{
    m_count_response = 0;
    m_t0 = 0;
}

CTestClientObj::~CTestClientObj()
{
}

bool CTestClientObj::InitSocketLib()
{
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

bool CTestClientObj::Init(const char* host, uint16_t port, uint32_t timeout)
{
    InitSocketLib();
    m_longconn = CreateLongConnObj();
    bool b = m_longconn ? m_longconn->InitLongConn(this, 1024, host, port,
                                                   static_cast<float>(timeout)) :false;
    return b;
}

void CTestClientObj::Uninit()
{
    if (m_longconn) {
        m_longconn->UninitLongConn();
        m_longconn->Release();
    }        
    m_longconn = 0;    
}

void CTestClientObj::OnTasksFinishedCallBack(LTasksGroup* task_group)
{
    if(m_t0 == 0)
        m_t0 = time(0);    

    uint32_t ok_count=0;
    for (uint32_t u = 0;u < task_group->m_valid_tasks; u++) {
        if (task_group->m_tasks[u]._is_send_ok && task_group->m_tasks[u]._is_receive_ok) {
            ok_count++;
        } else {
            const char* psz = 0;
            if (!task_group->m_tasks[u]._is_send_ok)
                psz = "connect fail.";
            else if (task_group->m_tasks[u]._is_send_ok && 
                    !task_group->m_tasks[u]._is_receive_ok)
                psz = "try receive data,time out";
            LOG(INFO) << "try get response fail:" << psz
                <<" on task" << u
                << ",request: max valid tasks:" << task_group->m_valid_tasks;
        }
    }

    if (ok_count == task_group->m_valid_tasks) {
        m_count_response++;
        if (m_count_response%500 == 0) {
            float favg = ((float)m_count_response)/(time(0)-m_t0);
            LOG(INFO) << "received response avg:" << favg
                << ", total received " << m_count_response;
        }
    }       
}

void CTestClientObj::OnUserRequest(LongConnHandle session,
                                   const unsigned char* data, uint32_t len,
                                   bool& need_response)
{
    LOG(INFO) << "client get user request.";
}

void CTestClientObj::OnClose(xfs::base::LongConnHandle) {
    LOG(INFO) << "client close";
}