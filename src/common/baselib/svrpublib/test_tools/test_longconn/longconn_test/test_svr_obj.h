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

    // long conn session received data,ֻ�������û������øô������Ƿ���л�Ӧ��,
    // ���ڻ�����ˮ������������µ�ѩ��ЧӦ,Ĭ��ֵneed_response=TRUE,
    // ��������Ӧ�������������need_response=FALSE
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
