#ifdef WIN32
#pragma   warning(disable:4127)
#endif // WIN32

#include "common/baselib/svrpublib/test_tools/test_svrpublib/head_file.h"

TestCOM* g_testcom = NULL;

CTestThread::CTestThread(void)
{
    CHECK(5);
}

CTestThread::~CTestThread(void)
{
}

void    CTestThread::Routine()    //  继承者必须实现这个函数
{
    LOG(INFO) << "CTestThread::Routine()\r\n";
    LOG(INFO) << "try stop routine()\r\n";
    g_testcom->AddRef();
    g_testcom->Release();
    StopRoutine();
}
