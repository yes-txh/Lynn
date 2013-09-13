#ifndef COMMON_BASELIB_SVRPUBLIB_TEST_TEST_TOOLS_LONGCONN_TEST_LONGCONN_SVR_THREAD_H_
#define COMMON_BASELIB_SVRPUBLIB_TEST_TEST_TOOLS_LONGCONN_TEST_LONGCONN_SVR_THREAD_H_


class CLongConnSvrThread:public CXThreadBase
{
public:
    CLongConnSvrThread();
    virtual ~CLongConnSvrThread();

    virtual void    Routine();//继承者必须实现这个函数

private:
    CTestSvrObj m_svr_obj;
};
#endif // COMMON_BASELIB_SVRPUBLIB_TEST_TEST_TOOLS_LONGCONN_TEST_LONGCONN_SVR_THREAD_H_
