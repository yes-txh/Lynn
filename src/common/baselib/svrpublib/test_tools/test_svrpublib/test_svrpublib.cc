// TestSvrpublib.cpp : Defines the entry point for the console application.
//

#include "common/baselib/svrpublib/test_tools/test_svrpublib/head_file.h"


void testLock(CXThreadMutex* mutex) {
    CXThreadAutoLock auto_lock(mutex);
    LOG(INFO) << "test mutex.";
}

void testReleaseMem(unsigned char* &mem) {
    delete []mem;
    mem = 0;
}

struct SampleNode {
    char a;
    unsigned short b;
    struct SampleNode* next;
};


template<typename R>
class _ReturnValueHolder {
public:
    _ReturnValueHolder(const R& value) : m_value(value) {
    }
    const R& Value() const {
        return m_value;
    }
private:
    R m_value;
};

template<typename R>
R GetVal(R val) {
    return val;
}

class CAA {
public:
    CAA(const int val): m_val(val) {
    }

    // const int& Value() const
    // {
    //   return m_val;
    // }
private:
    int m_val;
};

int GetIntVal(int val) {
    return val;
}


class _tagMyT {
public:
    _tagMyT* next;
    int a;
    _tagMyT() {
        next = NULL;
        a = 100;
    }

    ~_tagMyT() {}
};
typedef _tagMyT MyT;


struct MyTestType {
    char a;
    uint16_t b;
    int c;

    MyTestType() {
        a = 'a';
        b = 168;
        c = 1234;
    }

    void ToNetOrder() {
        b = H2NS(b);
        c = H2NL(c);
    }

    void ToHostOrder() {
        b = N2HS(b);
        c = N2HL(c);
    }

    void PrintVal() {
        LOG(INFO) << "a=" << a
            <<",b=" << b
            <<",c=" << c;
    }
};

int32_t get_test_val() {
    return 5;
}

DECLARE_USING_LOG_LEVEL_NAMESPACE;

void test_connect() {
    SOCKET sock = NewSocket(true);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("121.14.35.168");
    addr.sin_port = H2NS(80);

    socklen_t len = sizeof(addr);
    int32_t err = 0;
    err = connect(sock, (struct sockaddr*)&addr, len);

    bool b = ConnWriteable(sock, 3, &addr);

    if (!b) {
        LOG(ERROR) << "connect fail.";
    }
}

int main(int argc, char** argv) {
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);
    AutoBaseLib auto_baselib;
    AutoSocketStartup auto_sock_lib;   

    /*test_connect();

    // test safe_snprintf
    // const char* src="123";
    const char* src = NULL;
    char buf[3];
    int32_t n = safe_snprintf(buf, sizeof(buf), "%s", src);
    LOG(INFO) << "n=" << n << ",buf=" << buf;

    g_testcom = new TestCOM;
    g_testcom->AddRef();

    // CKeyValueParser parser;
    // bool is_ok=parser.ParserFromFile("c:\\a.ini");
    // unsigned char buf[128]={0};
    // for (int32_t ii=4;ii>=0;ii--)
    // {
    //    is_ok=parser.GetValue("a1",buf,sizeof(buf),ii);
    //    printf("index=%d,val=%s,is_ok=%d\r\n",ii,buf,is_ok);
    // }

    //
    // Set CHECK_xx ...
    //
    SetCheckErrLevel(ERR);
    SetCheckProgramName(argv[0]);

    CHECK_EQ(get_test_val(), 5);

    uint64_t uval = 987654321;
    int64_t nval = 1234567890L;
    LOG(INFO) << "uval = " << uval;
    LOG(INFO) << "nval = " << nval;

#ifdef WIN32
    FILE* fp = fopen("c:\\a.txt", "rb");

    if (fp) {
        int n = fseek(fp, 1024 * 1024, SEEK_SET);
        LOG(INFO) << "seek result:" << n;
        fclose(fp);
    }

#endif

    //    ConfigLOG(INFO,true,true,false,true);
    CBaseQ<MyT> queue;

    MyT* my_t_node = new MyT;
    queue.AppendNodeAtTail(my_t_node);

    queue.GetNodeOnHead(&my_t_node);

    //    Closure<void>* pCB=0;

    // AA<> objA=GetIntVal(234);
    CAA obj_a = GetIntVal(234);
    obj_a = 987;
    //    QASSERT(0);

    _ReturnValueHolder<int> result = GetVal(123);
    // result=GetVal(123);
    //   int val=(int)result;

    LongConnNode* node = mempool_NEW(LongConnNode);    
    mempool_DELETE(node);

    CVDataQueue_T<SampleNode>   data_queue;    

    // ----------------------------------------------------
    // 测试 0 key打包
    CBaseProtocolPack pack;
    pack.Init();

    pack.ResetContent();
    pack.SetServiceType(28);
    unsigned char* data = 0;
    uint32_t len = 0;
    pack.SetKey(2, "woo");
    pack.SetKey(1, 0);

    // 支持自定义数据类型
    MyTestType stMyTest;
    stMyTest.ToNetOrder();
    pack.SetKey(3, &stMyTest);
    MyTestType st_test[5];
    int i = 0;

    for (i = 0; i < 5; i++)
        st_test[i].ToNetOrder();

    pack.SetKey(4, (unsigned char*)&st_test, 5 * sizeof(MyTestType));

    char ch = 'x';
    pack.SetKey(5, ch);

    pack.GetPackage(&data, &len);

    LOG(INFO) << "________________________";
    bool first_time = true;

    // 第一次，检查数据来源
    if (first_time) {
        CHECK(IsValidPack(reinterpret_cast<BasProtocolHead*>(data)));

        if (!IsValidPack(reinterpret_cast<BasProtocolHead*>(data)))
            return 0;

        first_time = false;
    }

    uint32_t u = 0;
    // 测试XGUID
    XGUID uid;

    // for (u=0;u<50;u++)
    for (u = 0; u < 20; u++) {
        GetGUID(&uid);
        LOG(INFO) << "uuid:" << uid.data1 << "," << uid.data2 << ", "<< uid.data3 << ","
            << uid.data4[0] << "-" << uid.data4[1] << "-" 
            << uid.data4[2] << "-" << uid.data4[3] << "-"
            << uid.data4[4] << "-" << uid.data4[5] << "-" 
            << uid.data4[6] << "-" << uid.data4[7];
    }

    LOG(INFO) << "";


    CBaseProtocolUnpack unpack;
    unpack.Init();

    unpack.AttachPackage(data, len);
    bool b = unpack.Unpack();
    LOG(INFO) << "unpack result:" << (b ? "true" : "false");
    uint16_t st = unpack.GetServiceType();
    LOG(INFO) << "service type:" << st;
    // unsigned char* ptr;
    unsigned char* ptr;
    unpack.GetVal(2, &ptr, &len);

    // 获取自定义数据类型
    MyTestType* my_test = 0;

    if (unpack.GetVal(3, &my_test)) {
        my_test->ToHostOrder();
        my_test->PrintVal();
    }

    MyTestType* test_array = 0;
    int32_t items = 0;

    if (unpack.GetValArray(4, &test_array, &items)) {
        for (i = 0; i < static_cast<int>(items); i++) {
            test_array[i].ToHostOrder();
            test_array[i].PrintVal();
        }
    }

    int ich = 0;
    unpack.GetVal(5, &ich);

    pack.Uninit();
    //----------------------------------------------------

    //----------------------------------------------------
    // test lock
    CXThreadMutex mutex;
    testLock(&mutex);
    //----------------------------------------------------

    // test release memory
    unsigned char* mem = new unsigned char[1024];
    testReleaseMem(mem);

    // test mempool
    mem = mempool_NEW_BYTES(1024);
    mempool_DELETE(mem);

    mem = mempool_NEW_BYTES(1024);
    // 测试内存泄露
    // mempool_DELETE(pMem);*/


    // test thread
    CTestThread test_thread;
    // run once
    /*test_thread.StartThread();

    while (!test_thread.IsStopped()) {
        XUSleep(100);
    }*/

    test_thread.ForceEndThread();

    //g_testcom->Release();


    LOG(INFO) << "Hello World!";
    LOG(WARNING) << "test log level:warning.";
    LOG(ERROR) << "test log level:error.";
    LOG(INFO) << "test log level:info.";

    // 提前检测内存泄露
    // 如果不主动调用,进程退出会自动调用
    //mempool_Clean();

    return 0;
}

