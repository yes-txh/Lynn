// test_obj.h: interface for the CTestClientObj class.
//
//////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_TEST_TEST_TOOLS_LONGCONN_TEST_TEST_CLIENT_OBJ_H_
#define COMMON_BASELIB_SVRPUBLIB_TEST_TEST_TOOLS_LONGCONN_TEST_TEST_CLIENT_OBJ_H_
#include "common/baselib/svrpublib/server_publib_namespace.h"

class CTestClientObj:public ITasksGroupCallBack
{
public:
	CTestClientObj();
	virtual ~CTestClientObj();

    bool InitSocketLib();
    bool Init(const char* host, uint16_t port, uint32_t timeout);
    void Uninit();

    virtual void OnTasksFinishedCallBack(LTasksGroup* task_group);
    // long conn session received data,只读用于用户层设置该次请求是否会有回应包,
    // 用于缓减洪水般短连接请求下的雪崩效应,默认值need_response=TRUE,
    // 如果不想回应这次请求则设置need_response=FALSE
    virtual void OnUserRequest(LongConnHandle session,
                               const unsigned char* data, uint32_t len,
                               bool& need_response);

    virtual void OnClose(xfs::base::LongConnHandle);
    
    ILongConn*  m_longconn;
private:

    //
    // response count
    //
    uint32_t    m_count_response; 
    time_t      m_t0;
};  

#endif // COMMON_BASELIB_SVRPUBLIB_TEST_TEST_TOOLS_LONGCONN_TEST_TEST_CLIENT_OBJ_H_
