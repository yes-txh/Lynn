#ifndef SRC_XFS_BASELIB_SVRPUBLIB_TEST_TOOLS_TEST_SVRPUBLIB_TEST_THREAD_H_
#define SRC_XFS_BASELIB_SVRPUBLIB_TEST_TOOLS_TEST_SVRPUBLIB_TEST_THREAD_H_

using namespace xfs::base;

class TestCOM {
public:
    TestCOM() {
        m_count = 0;
        LOG(INFO) << "TestCOM()";
        // new ֮����ҪAddRef()
    }
    ~TestCOM() {
        LOG(INFO) << "~TestCOM()";
    }

    void AddRef() {
        InterlockedIncrement(&m_count);
    }

    void Release() {
        ATOM_INT val = InterlockedDecrement(&m_count);

        if (val == 0) {
            delete this;
        }
    }

    ATOM_INT m_count;
};

extern TestCOM* g_testcom;

class CTestThread : public CXThreadBase {
public:
    CTestThread(void);
    virtual ~CTestThread(void);

    virtual void    Routine();    //  �̳��߱���ʵ���������
};
#endif // SRC_XFS_BASELIB_SVRPUBLIB_TEST_TOOLS_TEST_SVRPUBLIB_TEST_THREAD_H_
