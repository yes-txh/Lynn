// lite_mempool.h: interface for the CMemPoolObj class.
// wookin@tencent.com
// 2010-05-07
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_LITE_MEMPOOL_H_
#define COMMON_BASELIB_SVRPUBLIB_LITE_MEMPOOL_H_

#include <deque>
#include <algorithm>
#include <new>
#include "common/baselib/svrpublib/wrapper_rwlock.h"
#include "common/baselib/svrpublib/base_config.h"
#include "thirdparty/glog/logging.h"

_START_XFS_BASE_NAMESPACE_

// //////////////////////////////
//
//
// New MemoryPool, Public interface
//
// mempool_NEW
// mempool_NEW_BYTES
// mempool_DELETE
//
// ---aux. function---
// mempool_Clean
// mempool_SetMemoryLimit
// mempool_Debug_IsValidPtr
//
// //////////////////////////////

//
// _DEBUG   模式下默认打开内存管理器内置的内存泄露检查
//          内存泄露检查生效, 启用内存泄露检查会增大内存分配尺寸
//

#define USE_MEMPOOL 1 // 如果该值为1,则使用mempool,否则直接new,delete

#if defined(_DEBUG)
//#define mempool_NEW(obj_type)   new(xfs::base::g_mempool_obj->GetMemory(sizeof(obj_type), __FILE__, __LINE__))obj_type
#define mempool_NEW(obj_type)   internal_mempool_new_fun<obj_type>(__FILE__, __LINE__)
    
#define mempool_NEW_BYTES(size) internal_mempool_new_bytes_fun<unsigned char>(size, __FILE__, __LINE__)

#define mempool_DELETE(obj)     internal_mempool_delete_fun(obj, __FILE__, __LINE__)
//#define mempool_NEW_BYTES(size) new(xfs::base::g_mempool_obj->GetMemory(size, __FILE__, __LINE__))unsigned char

#else // RELEASE
//#define mempool_NEW(obj_type)   new(xfs::base::g_mempool_obj->GetMemory(sizeof(obj_type)))obj_type

//#define mempool_NEW_BYTES(size) new(xfs::base::g_mempool_obj->GetMemory(size))unsigned char

#define mempool_NEW(obj_type)   internal_mempool_new_fun<obj_type>(NULL, 0)    
#define mempool_NEW_BYTES(size) internal_mempool_new_bytes_fun<unsigned char>(size, NULL, 0)

#define mempool_DELETE(obj)     internal_mempool_delete_fun(obj, NULL, 0)

#endif // defined(_DEBUG)

//
// 将内存放回内存池
// 会自动执行析构函数~T(),外部不需要显示执行析构函数,如调用*->~T();
//
//#define mempool_DELETE                  xfs::base::g_mempool_obj->PutMemory
//#define mempool_DELETE(obj)             internal_mempool_delete_fun(obj,)
#define mempool_Clean                   xfs::base::g_mempool_obj->Clean

//
//  mem_limit_size:bytes;
//  设置最大内存限制, 0:无限制。默认无限制.
//
#define mempool_SetMemoryLimit(mem_limit_size) \
    xfs::base::g_mempool_obj->SetMemLimitSize(mem_limit_size)

#define mempool_GetMemPoolSize                 \
    xfs::base::g_mempool_obj->GetMemPoolSize

//
//  超过这个尺寸的内存不进行cache, 每次用完就删除
//
#define MEMPOOL_MAX_CACHE_SIZE  10*1024*1024

// -----------------------------------------------------------------------

template<typename T>
class MemDualLink {
public:
    void Append(T* t);
    bool Empty();
    uint32_t Size() {
        return m_dual_link.size();
    }
    bool Delete(T* t);
    T* Front();
    void Clear();
private:
    std::deque<T*> m_dual_link;
    CXThreadMutex m_mutex;
};

template<typename T>
void MemDualLink<T>::Append(T *t) {
    CXThreadAutoLock auto_lock(&m_mutex);
    m_dual_link.push_back(t);
}

template<typename T>
bool MemDualLink<T>::Empty() {
    CXThreadAutoLock auto_lock(&m_mutex);
    return m_dual_link.empty();
}

template<typename T>
bool MemDualLink<T>::Delete(T* t) {
    CXThreadAutoLock auto_lock(&m_mutex);
    typename std::deque<T*>::iterator iter;
    iter = find(m_dual_link.begin(), m_dual_link.end(), t);
    if (iter == m_dual_link.end()) {
        return false;
    }
    m_dual_link.erase(iter);
    return true;
}

template<typename T>
T* MemDualLink<T>::Front() {
    CXThreadAutoLock auto_lock(&m_mutex);
    if (m_dual_link.size() == 0) {
        return NULL;
    }
    T* t = m_dual_link.front();
    m_dual_link.pop_front();
    return t;
}

template<typename T>
void MemDualLink<T>::Clear() {
    CXThreadAutoLock auto_lock(&m_mutex);
    m_dual_link.clear();
}

#ifdef USING_GOOGLE_MEMPOOL

// 内存段的头部，主要用来记录debug信息，内存对齐到8byte位置
struct MemSegDebugHead {
    uint8_t offset;
    unsigned char prob[7];
#if defined(_DEBUG)
    uint32_t        line;
    unsigned char   filename[36];
#endif
};

class GoogleMemPool {
public:
    GoogleMemPool() {};
    ~GoogleMemPool() {
        Clean();
    }
    void SetMemLimitSize(unsigned long max_mem_size) {};
    uint32_t GetMemPoolSize()const {
        return 0;
    }
    MemDualLink<unsigned char>* GetLink() {
        return &seg_link;
    }
    unsigned char*  GetMemory(uint32_t request_mem_len,
                              const char* file_name,uint32_t line);

    unsigned char*  GetMemory(uint32_t request_mem_len);

    template<typename T>
    void PutMemory(T* &obj_mem) {
        if ( obj_mem == NULL)
            return;

        obj_mem->~T();
        unsigned char*  pmem = (unsigned char*)obj_mem;
        PutMemoryBytes(pmem);
        obj_mem = NULL;
    }
    void Clean();
private:
    MemDualLink<unsigned char> seg_link;
    void PutMemoryBytes(unsigned char*);
};

extern GoogleMemPool* g_mempool_obj;

#else //USING_GOOGLE_MEMPOOL
//
// check is valid pointer
//
#define mempool_Debug_IsValidPtr(ptr) \
    mempool_debug_is_valid_ptr(__FILE__, __LINE__, ptr)


// ----------------------------------------------------------------------------
//
// 比较友好的做法是在程序main()函数中最后退出之前主动调用mempool_Clean(),
// 会提前知道内存泄露.
//
// 如果不主动调用mempool_Clean(), SvrPubLib库在析构之前也会释放内存管理器,
// 其中也会调用mempool_Clean()
//
// ----------------------------------------------------------------------------



// /////////////////////////////////
// 测试代码
//    //申请节点
//    myTestNode* ptr= mempool_NEW(myTestNode);//不需要预先定义, 直接使用
//    ptr->a;
//    ptr->b;
//    // mempool_DELETE()会主动调用析构函数
//    mempool_DELETE(ptr);
//
//    CMy* my_obj = mempool_NEW(CMy);
//    my_obj->hi();
//    // mempool_DELETE()会主动调用析构函数
//    mempool_DELETE(my_obj);
//
//    // random test
//    srand(uint32_t(time(0)));
//    for (int32_t i = 0;i<1000000;i++)
//    {
//        int32_t nLen = rand();
//        unsigned char*  mptr = mempool_NEW_BYTES(nLen);
//        mempool_DELETE(mptr);
//    }
//
//      mempool_Clean();
//
// /////////////////////////////////

//
// 定义为每一块mem block一次分配多少个节点,
// NodeSize小于1024的时候每次新增100个节点,
// 大于1024小于128k每次每次新增10个节点, 大于128k, 每次新增1个节点
//
// 每块memChunk总空间大小为 = 节点数*(external_head+节点大小)
//
#define MAX_MEMP_NODES(bytes_per_node)                  \
            ((bytes_per_node> 1024)              ?     \
            ((bytes_per_node > 1024*128) ? 1:10) : 100)


//
// 向上对齐到MEMPOOL_ROUND_SIZE的整数倍
//
#define FIXED_NODE_SIZE_TO_ROUND_SIZE_N(bytes)            \
        (((bytes%MEMPOOL_ROUND_SIZE(bytes)) == 0) ?              \
                                     bytes :              \
        ((1+bytes/MEMPOOL_ROUND_SIZE(bytes))*MEMPOOL_ROUND_SIZE(bytes)))

// 如果当前列表的没有空闲节点，允许搜索内存块长度更大的列表
#define MAX_HOP_SIZE(bytes_per_node) \
    ((bytes_per_node> 1024) ? 2*(bytes_per_node) : 1024)


#define MEMP_SegmentState_FREE    '-'
#define MEMP_SegmentState_INUSE   '*'

struct MEMP_MemItem;
struct MEMP_MemChunk;
class CMemPoolObj {
public:
    void Debug();

    CMemPoolObj();
    virtual ~CMemPoolObj();

    void Clean();
    void SetMemLimitSize(unsigned long max_mem_size); 
    

    unsigned char* GetMemory(uint32_t mem_len, const char* file_name,
                             uint32_t line);  

    //
    //  获取指定长的内存块(有可能返回的内存大于等于要求的内存长度)
    //
    unsigned char* GetMemory(uint32_t request_mem_len);

    //
    // 将内存放回内存池
    // 会自动执行析构函数~T(),外部不需要显示执行析构函数,如调用*->~T();
    //
    template<typename T>
    void PutMemory(T* &obj_mem) {
        if ( obj_mem == NULL)
            return;

        obj_mem->~T();
        unsigned char* pmem = (unsigned char*)obj_mem;
        PutMemoryBytes(pmem);
        obj_mem = NULL;
    }

    uint32_t GetMemPoolSize()const;
    void GarbageCollection();

private:
    void FreeChunk(MEMP_MemChunk*, uint32_t seg_size);
    unsigned long volatile  m_mempool_limit_size; // 限制总共最大能分配多少内存
    // 0:无限制

    unsigned long volatile  m_alloced_mem_count;  // 已经分配出来的内存大小
    unsigned long volatile  m_freed_mem_count;    // 已经被用户释放的内存大小
    // 也就是现在空闲列表中内存大小
    //
    // MEMP_MemItem*
    //
    MEMP_MemItem** volatile m_mem_items;          // 内存数组
    uint32_t volatile       m_items_array_len;    // 内存数组长度
    uint32_t volatile       m_valid_items;        // 内存数组有效长度

    CXThreadMutex           m_mutex;              // 支持多线程, 需要加锁
    RWLocker                m_rwlock;
    //
    //  1:查找第一次出现大于等于MemSize的位置(MemSize是要求要分配的大小)
    //  2:bAutoInsert:如果不存在是否自动插入
    //
    bool FindMemItemPos(uint32_t mem_size,  MEMP_MemItem** mem_item,
                        bool auto_insert);

    void PutMemoryBytes(unsigned char* &ptr_mem);
};

void mempool_debug_is_valid_ptr(const char* file, const int32_t line,
                                unsigned char* ptr_mem);

//
// 只在PublicObjs.cpp中实例化一个对象
//

extern CMemPoolObj*  g_mempool_obj;

//
// 在Clean的时候可以设置该回调函数,用于传回泄漏的地址
//
typedef void (*MEMLEAK_CALLBACK)(void* leak_addr);
void SetMemLeakCallback(MEMLEAK_CALLBACK cb);

//
// 定义具体的new,delete 函数(杜绝new数组对象)
// 用户可以通过定义USE_MEMPOOLUSE_MEMPOOL的值来决定使用mempool还是真实的new
//
// -----------------------------------------------------------------------
template<typename T>
T* internal_mempool_new_fun(const char* file, uint32_t line){    
#if USE_MEMPOOL
    unsigned char* mem_addr = xfs::base::g_mempool_obj->GetMemory(sizeof(T), file, line);
    T* obj = new(mem_addr)T;
    // 有可能上一步操作中有些对象的地址偏移变掉了,我们不支持变地址的情况
    bool is_eq = (mem_addr == (unsigned char*)(obj));
	if(!is_eq){
        abort();
        // LOG(google::FATAL) << "mem_addr:" << (void*)(mem_addr) << " != " << (void*)(obj);
    }
    return obj;
#else    
    T* obj = new T;
    return obj;
#endif   
}

template<typename T>
T* internal_mempool_new_bytes_fun(uint32_t size, const char* file, uint32_t line){
#if USE_MEMPOOL
    T* obj = new(xfs::base::g_mempool_obj->GetMemory(size, file, line))T;
    return obj;
#else    
    T* obj = new T[size];
    return obj;
#endif   
}


template<typename T>
void internal_mempool_delete_fun(T* &obj, const char* file, uint32_t line){
#if USE_MEMPOOL
    xfs::base::g_mempool_obj->PutMemory(obj);
#else
    delete obj;
    obj = NULL;
#endif
}

#endif // USING_GOOGLE_MEMPOOL

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_LITE_MEMPOOL_H_

