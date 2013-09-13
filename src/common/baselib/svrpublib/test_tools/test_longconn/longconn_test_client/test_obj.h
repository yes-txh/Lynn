// test_obj.h: interface for the CTestObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(XFS_TOOLS_BASIC_TOOLS_FRAMEWORK_TEST_TOOLS_LONGCONN_TEST_CLIENT_TEST_OBJ_H_)
#define XFS_TOOLS_BASIC_TOOLS_FRAMEWORK_TEST_TOOLS_LONGCONN_TEST_CLIENT_TEST_OBJ_H_

class CTestObj:public ITasksGroupCallBack
{
public:
	CTestObj();
	virtual ~CTestObj();

    bool Init(char* host, unsigned short port, unsigned int timeout);
    void Uninit();

    virtual void OnTasksFinishedCallBack(LTasksGroup* task_group);
    virtual void OnUserRequest(LongConnHandle session,                  // long conn session
                               const unsigned char* data, uint32_t len,  // received data,只读
                               bool& need_response                      // 用于用户层设置该次请求是否会有回应包,
                                                                        // 用于缓减洪水般短连接请求下的雪崩效应
                                                                        // 默认值bWillResponse=TRUE,
                                                                        // 如果不想回应这次请求则设置bWillResponse=FALSE
                               );

    virtual void OnClose(LongConnHandle lc_handle);
    
    ILongConn*  m_longconn;
private:

    //
    // response count
    //
    uint32_t    m_count_response; 
    time_t      m_t0;
};  

#endif // !defined(XFS_TOOLS_BASIC_TOOLS_FRAMEWORK_TEST_TOOLS_LONGCONN_TEST_CLIENT_TEST_OBJ_H_)
