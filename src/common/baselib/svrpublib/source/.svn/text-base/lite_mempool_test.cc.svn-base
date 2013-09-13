//////////////////////////////////////////////////////////////////////////
// lite_mempool_test.cc
// @brief:     Test Macros in lite_mempool.cc
// @author:  fatliu@tencent
// @time:     2010-10-28
// @version: 1.0
//////////////////////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/server_publib.h"
#include "thirdparty/gtest/gtest.h"

DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;

// class with data typed int
class CWithInt {
public:
    CWithInt() {
        m_data = 0;
    }
    explicit CWithInt(int32_t data) {
        m_data = data;
    }
    ~CWithInt() {
        m_data = 0;
    }

    int32_t m_data;
};

// class with ptr data managed by MemPool
class CWithMemPoolPtr {
public:
    CWithMemPoolPtr() {
        m_ptr = mempool_NEW_BYTES(64);
        memcpy(m_ptr, "default constructor...\r\n", 64);
    }
    explicit CWithMemPoolPtr(const char* data) {
        m_ptr = mempool_NEW_BYTES(64);
        memcpy(m_ptr, data, 64);
    }
    // 需要手动调用
    ~CWithMemPoolPtr() {
        // delete m_ptr;
        mempool_DELETE(m_ptr);
    }

    unsigned char* m_ptr;
};

// class with ptr data not managed by MemPool
class CWithPtr {
public:
    CWithPtr() {
        m_ptr = new char[64];
        memcpy(m_ptr, "default constructor...\r\n", 64);
    }
    explicit CWithPtr(const char* data) {
        m_ptr = new char[64];
        memcpy(m_ptr, data, 64);
    }

    // 不用手动调用
    ~CWithPtr() {
        delete[] m_ptr;
    }

    char* m_ptr;
};

#ifdef WIN32
int32_t TestLiteMempool(int32_t argc, char** argv)
#else
int32_t main(int32_t argc, char** argv)
#endif
{
#ifndef WIN32
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);

    AutoBaseLib auto_baselib;
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
#else
    return 0;
#endif
}

// @brief:     Test macro mempool_SetMemoryLimit
TEST(TestSetLimit, SetLimit) {
    mempool_SetMemoryLimit(3);
    unsigned char* mem_obj = mempool_NEW_BYTES(7);
    // 这里不一定能保证分配不到内存
    // EXPECT_TRUE(!mem_obj);

    mempool_DELETE(mem_obj);
    mempool_SetMemoryLimit(0);
}

// @brief:     Test macro mempool_NEW_BYTES
TEST(TestBytes, SomeBytes) {
    unsigned char* mem_test1 = mempool_NEW_BYTES(7);
    CHECK(mem_test1);
    unsigned char* mem_test2 = mempool_NEW_BYTES(MEMPOOL_MAX_CACHE_SIZE + 7);
    CHECK(mem_test2);

    uint32_t mem_size = mempool_GetMemPoolSize();
    CHECK(mem_size);

    mempool_DELETE(mem_test1);
    CHECK(!mem_test1);
    mempool_DELETE(mem_test2);
    CHECK(!mem_test2);
}

// @brief:     Test macro mempool_NEW with CWithInt
TEST(TestIntObj, IntObj) {
    CWithInt* mem_obj =  mempool_NEW(CWithInt);
    CHECK(mem_obj);    
    mempool_DELETE(mem_obj);
    CHECK(!mem_obj);
}

// @brief:     Test macro mempool_NEW with CWithPtr
TEST(TestPtrObj, PtrObj) {
    CWithPtr* mem_obj = mempool_NEW(CWithPtr);
    CHECK(mem_obj);    
    mempool_DELETE(mem_obj);
    CHECK(!mem_obj);
}

// @brief:     Test macro mempool_NEW with CWithMemPoolPtr
TEST(TestPtrMempoolObj, PtrMempoolObj) {
    CWithMemPoolPtr* mem_obj =
        mempool_NEW(CWithMemPoolPtr);

    CHECK(mem_obj);    
    mempool_DELETE(mem_obj);
    CHECK(!mem_obj);
}

// @brief:     Test macro mempool_NEW_BYTES with rand bytes
TEST(TestBytes, RandBytes) {
    srand(static_cast<uint32_t>(time(0)));
#ifdef _DEBUG
    for (int32_t i = 1; i < 1000000; i++) {
        int32_t len = safe_rand()+1;
        unsigned char* mem_test1 = mempool_NEW_BYTES(len);
        mempool_DELETE(mem_test1);
    }
#endif
}

// @brief:     Test case leak
void LeakTest() {
    CWithMemPoolPtr* mem_obj = new CWithMemPoolPtr();
    CHECK(mem_obj);
    // leak...
    mempool_DELETE(mem_obj);
    CHECK(!mem_obj);
}

// @brief:     Test case over flow
void OverFlowTest() {
    unsigned char* mem_obj = mempool_NEW_BYTES(7);
    CHECK(mem_obj);
    memset(mem_obj, 1, 8);
    // detect overflow
    mempool_DELETE(mem_obj);
    CHECK(!mem_obj);
}

// @brief:     death test for case leak and over flow
TEST(MyDeathTest, Death) {
#ifdef _DEBUG
    EXPECT_DEATH(OverFlowTest(), "");
    EXPECT_DEATH(LeakTest(), "");
#endif
}


int32_t g_max = 10;
void* g_ptr_array[ 10 * 2];
void MemLeakCallback(void* leak_addr){
	LOG(INFO) << "callback...";
    for(int32_t i = 0; i < (g_max * 2); ++i){
        if(g_ptr_array[i] == leak_addr){
            g_ptr_array[i] = NULL;
            break;
        }
    }
}


TEST(TestMemoryLeak, test_log_out_memory_leak_addr){
    srand(static_cast<uint32_t>(time(NULL)));
    int32_t i = 0;
    int32_t count = 0;
    for(i = 0; i < g_max; ++i){
        int32_t len = rand() % 65535;
        unsigned char* p1 = mempool_NEW_BYTES(len);
        unsigned char* p2 = mempool_NEW_BYTES(len);
        g_ptr_array[count] = p1; ++count;
        g_ptr_array[count] = p2; ++count;
        LOG(INFO) << "new memory len:" << len
                  << " p1:" << reinterpret_cast<void*>(p1)
                  << " , p2=" << reinterpret_cast<void*>(p2);
    };

    SetMemLeakCallback(MemLeakCallback);
	LOG(WARNING) << "mempool_Clean()...";
    mempool_Clean();
    
    // check result(only with _DEBUG, blade -t -p debug)
#ifndef NDEBUG
    for(i = 0; i < g_max; ++i){
        EXPECT_TRUE(g_ptr_array[i] == NULL);
        LOG(INFO) << "g_ptr_array[" << i << "]=" << g_ptr_array[i];
    }
#endif //
}

