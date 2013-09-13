// TestObj.cpp: implementation of the CTestObj class.
//
//////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test_svr/head_files.h"

using namespace xfs::base;
#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test_svr/test_obj.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTestObj::CTestObj() {
}

CTestObj::~CTestObj() {
}

bool CTestObj::Init(const char* host, unsigned short port, unsigned int timeout) {
    m_longconn = CreateLongConnObj();
    bool b = m_longconn?m_longconn->InitLongConn(this,
            1024,
            host,
            port,
            (float)timeout):false;

    m_pack.Init();
    m_unpack.Init();

    return b;
}

void CTestObj::Uninit() {
    if(m_longconn) {
        m_longconn->UninitLongConn();
        m_longconn->Release();
    }
    m_longconn = 0;

    m_pack.Uninit();
    m_unpack.Uninit();
}

uint32_t g_num_count = 0;
time_t g_t0;
void CTestObj::OnTasksFinishedCallBack(LTasksGroup* task_group) {
    if (g_num_count == 0)
        g_t0 = time(0);
    g_num_count++;

    uint32_t ok_count = 0;
    for(uint32_t u=0; u<task_group->m_valid_tasks; u++) {
        if(task_group->m_tasks[u].IsTaskFinished()) {
            ok_count++;
            //Xprintf_MemDebug(((LongConnNode*)(pTasksgrp->m_Tasks[u]._pNode))->data.szBuff,
            //                ((LongConnNode*)(pTasksgrp->m_Tasks[u]._pNode))->data.uValidLen);
        }
    }
    LOG(INFO) << "***total ok: " << ok_count << "***";

    if(g_num_count%5000==0) {
        float favg=((float)g_num_count)/(time(0)-g_t0);
        LOG(INFO) << "avg:" << favg;
    }
}

time_t g_s_t0 = 0;
unsigned int g_su_count = 0;

void CTestObj::OnUserRequest(LongConnHandle session,                // Long conn session
                             const unsigned char* data,uint32_t len,  // received data,只读
                             bool& need_response  // 用于用户层设置该次请求是否会有回应包,
                             // 用于缓减洪水般短连接请求下的雪崩效应
                             // 默认值need_response=true,
                             // 如果不想回应这次请求则设置need_response=false
                            ) {
    //Xprintf("get user request.\r\n");

#define TEST_SIMU_SERVER 1

#ifdef TEST_SIMU_SERVER

    LTasksGroup task;

    m_unpack.AttachPackage((unsigned char*)data,len);
    Xprintf_MemDebug((const char*)data,len);
    if(m_unpack.Unpack()) {
        // prepare response data
        // prepare data
        uint32_t seq=m_unpack.GetSeq();
        const char* msg=":),response from server,test is ok!";
        m_pack.ResetContent();
        // m_pack.SetOption(BASEPROTOCOL_OPT_IS_REQ_PACKAGE,true);
        m_pack.SetSeq(seq); // set seq
        m_pack.SetServiceType(1028);//SS_SEARCH2SB_RSP
        m_pack.SetKey(123,msg);

        // get response package
        unsigned char* pack_data=0;
        uint32_t pack_len=0;
        m_pack.GetPackage(&pack_data,&pack_len);

        // set send package
        task.m_tasks[0].SetSendData(pack_data,pack_len);
        // set session
        task.m_tasks[0].SetConnSession(session);
        // ? need response
        task.m_tasks[0].SetNeedResponse(1);

        // set valid tasks in group
        task.SetValidTasks(1);

        // send data
        if(!m_longconn->SendData(&task)) {
            LOG(ERROR) << "send response data to client fail.uSeq=" << seq;
        }
    } else {
        LOG(ERROR) << "***ERROR,Level 3***received user request,but unpack fail,pack len:"
                   << len;
    }

#endif//    

    if(g_su_count == 0)
        g_s_t0=time(0);
    g_su_count++;
    if(g_su_count%10000 == 0) {
        float favg = float(g_su_count)/(time(0)-g_s_t0);
        LOG(INFO) << "server, response avg: " << favg << " times/s";
    }
}

void CTestObj::OnClose(LongConnHandle lc_handle) {
    LOG(INFO) << "CTestObj::OnClose";
}
