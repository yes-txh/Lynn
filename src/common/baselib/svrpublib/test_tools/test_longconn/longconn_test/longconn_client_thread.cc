#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test/head_files.h"
DECLARE_USING_LOG_LEVEL_NAMESPACE;

DECLARE_int32(to_port);
DECLARE_string(to_host);

CLongConnClientThread::CLongConnClientThread() {
    LOG(INFO) << "long conn. test, *client* side!";
    uint32_t    timeout = 2; // seconds

    if (!m_client_obj.Init(NULL, 0, timeout)) {
        LOG(ERROR) << "init fail...";
        return;
    }

    m_session1 =
        m_client_obj.m_longconn->CreateLongConnSession(FLAGS_to_host.c_str(), FLAGS_to_port);
    m_session2 =
        m_client_obj.m_longconn->CreateLongConnSession(FLAGS_to_host.c_str(), FLAGS_to_port);
}

CLongConnClientThread::~CLongConnClientThread() {
    m_client_obj.m_longconn->RemoveLongConnSession(m_session1);
    m_client_obj.m_longconn->RemoveLongConnSession(m_session2);
    m_client_obj.m_longconn->RoutineLongConn(1000);       
    m_client_obj.Uninit();
    LOG(INFO) << "long connect client exit";
}

void CLongConnClientThread::Routine() {
    
    CBaseProtocolPack pack;
    pack.Init();
    // connect to
    

    uint32_t count = 0;
    uint32_t seq = 0;

    bool is_test_continue = true;

    int32_t milli_sleep_time = 100;
    for (int n=0; n<10; n++) {        
        LTasksGroup tasks;      
        seq++;
        // prepare request package        
        pack.ResetContent();
        pack.SetSeq(seq);
        pack.SetServiceType(500);

        char buff[128] = {0};
        safe_snprintf(buff,sizeof(buff),"test send ok count:%u :)",count);            
        pack.SetKey(100,(char*)buff);

        unsigned char* data = 0;
        uint32_t len = 0;
        pack.GetPackage(&data, &len);        

        // set task parameter            
        // set send package
        // set task 1
        tasks.m_tasks[0].SetSendData(data, len);
        // set session
        tasks.m_tasks[0].SetConnSession(m_session1);
        // need recv response       
        tasks.m_tasks[0].SetNeedResponse(1);


        // set task 2
        tasks.m_tasks[1].SetSendData(data, len);
        // set session
        tasks.m_tasks[1].SetConnSession(m_session2);
        // need recv response       
        tasks.m_tasks[1].SetNeedResponse(1);

        // set group parameters
        tasks.SetValidTasks(2);
        //Tasks.SetValidTasks(1);

        // send data        
        if (is_test_continue && !m_client_obj.m_longconn->SendData(&tasks)){
            LOG(INFO) << "long conn. test client,send data fail.seq=" << seq
                      << "sent ok count:" << count;
#ifdef WIN32
            Sleep(milli_sleep_time);
#else
            usleep(milli_sleep_time*1000);
#endif//
        } else
            count++;

        m_client_obj.m_longconn->RoutineLongConn(1000);        
    }

    pack.Uninit();

    return;
}
