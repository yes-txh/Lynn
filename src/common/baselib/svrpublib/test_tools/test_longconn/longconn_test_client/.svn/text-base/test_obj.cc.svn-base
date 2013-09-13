// TestObj.cpp: implementation of the CTestObj class.
//
//////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test_client/head_files.h"
using namespace xfs::base;
#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test_client/test_obj.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTestObj::CTestObj() {
    m_count_response = 0;
    m_t0 = 0;
}

CTestObj::~CTestObj() {
}

bool CTestObj::Init(char* host, unsigned short port, unsigned int timeout) {
    m_longconn = CreateLongConnObj();
    bool b = m_longconn ? m_longconn->InitLongConn(this,
                                              1024,
                                              host,
                                              port,
                                              (float)timeout)
                                              :false;
    return b;
}

void CTestObj::Uninit() {
    if (m_longconn) {
        m_longconn->UninitLongConn();
        m_longconn->Release();
    }        
    m_longconn = 0;    
}

void CTestObj::OnTasksFinishedCallBack(LTasksGroup* task_group) {
    if(m_t0 == 0)
        m_t0 = time(0);    

    uint32_t ok_count=0;
    for (uint32_t u = 0; u < task_group->m_valid_tasks; u++) {
        if (task_group->m_tasks[u]._is_send_ok && task_group->m_tasks[u]._is_receive_ok) {
            ok_count++;
        } else {
            const char* psz=0;
            if (!task_group->m_tasks[u]._is_send_ok)
                psz="connect fail.";
            else if (task_group->m_tasks[u]._is_send_ok && 
                    !task_group->m_tasks[u]._is_receive_ok)
                psz="try receive data,time out";
            LOG(INFO) << "try get response fail:" << psz << " ,on task" << u
                    << ",request: max valid tasks:" << task_group->m_valid_tasks;
        }
    }

    if (ok_count == task_group->m_valid_tasks) {
        m_count_response++;
        if (m_count_response % 500 == 0){
            float favg = ((float)m_count_response) / (time(0) - m_t0);
            LOG(INFO) << "received response avg:" <<favg
                <<" ,total received " << m_count_response;
        }
    }       
}

void CTestObj::OnUserRequest(LongConnHandle session,                    // long conn session
                             const unsigned char* data, uint32_t len,   // received data,只读
                             bool& need_response  // 用于用户层设置该次请求是否会有回应包,
                                                  // 用于缓减洪水般短连接请求下的雪崩效应
                                                  // 默认值bWillResponse=TRUE,
                                                  // 如果不想回应这次请求则设置
                                                  // bWillResponse=FALSE
                             ) {
    LOG(INFO) << "get user request.";

#ifdef TEST_SIMU_SERVER
    LTasksGroup task;
    
    m_oUnpack.AttachPackage(data,len);
    if (m_oUnpack.Unpack()) {
        // prepare response data
        // prepare data
        uint32_t seq = m_oUnpack.GetSeq();
        const char* psz = ":),response from server,test is ok!";
        m_oPack.ResetContent();
        m_oPack.SetSeq(seq); // set seq
        m_oPack.SetServiceType(100);
        m_oPack.SetKey(123, psz);        
        
        // get response package
        unsigned char* pack = 0;
        uint32_t pack_len = 0;
        m_oPack.GetPackage(&pack, &pack_len);
        
        // set send package
        task.m_Tasks[0].SetSendData(pack, pack_len);
        // set session
        task.m_Tasks[0].SetConnSession(session);
        // ? need response
        task.m_Tasks[0].SetNeedResponse(0);
        
        // set valid tasks in group
        task.SetValidTasks(1);        
        
        // send data
        if (!m_longconn->SendData(&task)) {
            LOG(ERROR) << "***ERROR,Level 3***,"
                    << "send response data to client fail.uSeq=" << seq;
        }        
    } else {
        LOG(ERROR) << "***ERROR,Level 3***,"
               << "received user request,but unpack fail,pack len:" << len;
    } 
#endif//    
}

void CTestObj::OnClose(LongConnHandle lc_handle) {
    LOG(INFO) << "CTestObj::OnClose";
}