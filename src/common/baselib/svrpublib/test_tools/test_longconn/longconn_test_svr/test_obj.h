// test_obj.h: interface for the CTestObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(XFS_TOOLS_BASIC_TOOLS_FRAMEWORK_TEST_TOOLS_LONGCONN_TEST_SVR_TEST_OBJ_H_)
#define XFS_TOOLS_BASIC_TOOLS_FRAMEWORK_TEST_TOOLS_LONGCONN_TEST_SVR_TEST_OBJ_H_

class CTestObj:public ITasksGroupCallBack {
public:
    CTestObj();
    virtual ~CTestObj();

    bool Init(const char* host,unsigned short port,unsigned int timeout);
    void Uninit();

    virtual void OnTasksFinishedCallBack(LTasksGroup* task_group);

    virtual void OnUserRequest(LongConnHandle session,                  // Long conn session
                               const unsigned char* data, uint32_t len,  // received data,ֻ��
                               bool& need_response   // �����û������øô������Ƿ���л�Ӧ��,
                               // ���ڻ�����ˮ������������µ�ѩ��ЧӦ
                               // Ĭ��ֵneed_response=true,
                               // ��������Ӧ�������������need_response=false
                              );

    virtual void OnClose(LongConnHandle lc_handle);


    ILongConn*          m_longconn;
private:
    CBaseProtocolPack   m_pack;
    CBaseProtocolUnpack m_unpack;
};

#endif // !defined(XFS_TOOLS_BASIC_TOOLS_FRAMEWORK_TEST_TOOLS_LONGCONN_TEST_SVR_TEST_OBJ_H_)
