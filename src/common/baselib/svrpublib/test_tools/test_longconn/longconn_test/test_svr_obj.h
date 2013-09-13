// test_obj.h: interface for the CTestSvrObj class.
//
//////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_TEST_TEST_TOOLS_LONGCONN_TEST_TEST_SVR_OBJ_H_
#define COMMON_BASELIB_SVRPUBLIB_TEST_TEST_TOOLS_LONGCONN_TEST_TEST_SVR_OBJ_H_

class CTestSvrObj:public ITasksGroupCallBack {
public:
	CTestSvrObj();
	virtual ~CTestSvrObj();


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

    ILongConn*          m_longconn;
private:
    CBaseProtocolPack   m_pack;
    CBaseProtocolUnpack m_unpack;
};

#endif // COMMON_BASELIB_SVRPUBLIB_TEST_TEST_TOOLS_LONGCONN_TEST_TEST_SVR_OBJ_H_
