// LongConnTest.cpp : Defines the entry point for the console application.
//
#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test_client/head_files.h"
using namespace xfs::base;
#include "common/baselib/svrpublib/test_tools/test_longconn/longconn_test_client/test_obj.h"

bool InitSocketLib(){
    bool b = false;
#ifdef WIN32
	WSADATA wsaData = {0};
    WORD wVersionRequested = MAKEWORD(2, 2 );
    int32_t err = WSAStartup(wVersionRequested, &wsaData );
    if ( err != 0 ) {
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
	b=true;
#endif //_LINUX_OS_
	return b;
}
DECLARE_USING_LOG_LEVEL_NAMESPACE;
DEFINE_int32(to_port, 62500, "long connection server port");
DEFINE_string(to_host, "127.0.0.101", "connect to host,ip");

int main(int argc, char* argv[]) {
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, false);

    LOG(INFO) << "long conn. test, *client* side!";
    LOG(INFO) << "usage:" << argv[0] << " conn. to host,port";

    // get host,port
    const char*    server_host = FLAGS_to_host.c_str();
    uint16_t       port = FLAGS_to_port;
    uint32_t       timeout = 2; // seconds
    LOG(INFO) << "host: " << server_host << " port:" << port;
    // init stack lib on WIN32
    InitSocketLib();
    AutoBaseLib abl;
    CBaseProtocolPack pack;
    pack.Init();

    CTestObj obj;
    if (!obj.Init(NULL, 0, timeout)) {
        LOG(ERROR) << "init test object fail...";
        return -1;
    }

    // connect to
    LongConnHandle session1 = obj.m_longconn->CreateLongConnSession(server_host, port);
    LongConnHandle session2 = obj.m_longconn->CreateLongConnSession(server_host, port);

    uint32_t count = 0;
    uint32_t seq = 0;

    bool is_test_continue = true;

    int32_t milli_sleep_time = 100;
    //bool bContinue=true;
    //while(bContinue)
    for (int n = 0; n < 100; n++) {
        LTasksGroup tasks;
        seq++;
        // prepare request package
        pack.ResetContent();
        pack.SetSeq(seq);
        pack.SetServiceType(500);

        char buff[128] = {0};
        safe_snprintf(buff, sizeof(buff), "test send ok count:%u :)", count);
        pack.SetKey(100, static_cast<char*>(buff));

        unsigned char* data=0;
        uint32_t len=0;
        pack.GetPackage(&data, &len);

        // set task parameter
        // set send package
        // set task 1
        tasks.m_tasks[0].SetSendData(data, len);
        // set session
        tasks.m_tasks[0].SetConnSession(session1);
        // need recv response
        tasks.m_tasks[0].SetNeedResponse(1);


        // set task 2
        tasks.m_tasks[1].SetSendData(data, len);
        // set session
        tasks.m_tasks[1].SetConnSession(session2);
        // need recv response
        tasks.m_tasks[1].SetNeedResponse(1);

        // set group parameters
        tasks.SetValidTasks(2);
        //Tasks.SetValidTasks(1);

        // send data
        if (is_test_continue && !obj.m_longconn->SendData(&tasks)) {
            LOG(INFO) << "long conn. test client,send data fail.uSeq="
                    << seq << ",sent ok count:" << count;
#ifdef WIN32
        Sleep(milli_sleep_time);
#else
        usleep(milli_sleep_time*1000);
#endif//
        }
        else
            count++;

        obj.m_longconn->RoutineLongConn(1000);
        XSleep(1000);
    }

    obj.m_longconn->RemoveLongConnSession(session1);
    obj.m_longconn->RemoveLongConnSession(session2);

    // 主动调用Routine,删除长连接,LongConnObject->Routine()
    // 调用不是同一个线程则不能调用这个
    obj.m_longconn->RoutineLongConn(1000);

    obj.Uninit();
    pack.Uninit();

	return 0;
}
