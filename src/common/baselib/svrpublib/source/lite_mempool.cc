//
//  lite_mempool.cc
//  wookin@tencent.com
/////////////////////////////////////////////////////////

#include "common/baselib/svrpublib/server_publib.h"

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

//
// 检查内存越界的填充
//
#define MEM_DETECT_VAL 0xB0

#ifdef USING_GOOGLE_MEMPOOL
void GoogleMemPool::Clean() {
    while (1) {
        unsigned char*  seg_start = GetLink()->Front();
        if(!seg_start)
            return;
#if defined(_DEBUG)
        MemSegDebugHead* debug_head = (MemSegDebugHead*)seg_start;
        LOG(ERROR) << "Detected memory leak in file:" <<
                   debug_head->filename << ", at line:" << debug_head->line;
#endif
        free(seg_start);
    }
}

unsigned char* GoogleMemPool::GetMemory(uint32_t request_mem_len, const char* file_name,uint32_t line) {
    unsigned char* mem_start = GetMemory(request_mem_len);
    if (!mem_start) return mem_start;

    MemSegDebugHead* seg_head = (MemSegDebugHead*)(mem_start - sizeof(MemSegDebugHead));
    seg_head->line = line;
    const char* name_start =
        strrchr(reinterpret_cast<const char*>(file_name),
                '/');

    if (!name_start)
        name_start = strrchr(reinterpret_cast<const char*>(file_name),
                             '\\');
    if (name_start)
        name_start++;
    else
        name_start = (const char*)file_name;
    uint32_t len = STRLEN(name_start);
    len = (len >= sizeof(seg_head->filename)) ?
          sizeof(seg_head->filename) - 1 :
          len;

    memcpy(seg_head->filename, name_start, len);
    seg_head->filename[len] = 0;

    if (seg_head->line == 0) {
        LOG(FATAL) << "error,line_num = " << line;
    }
    return mem_start;
}

unsigned char*  GoogleMemPool::GetMemory(uint32_t request_mem_len) {
    uint32_t real_mem_len = request_mem_len + sizeof(MemSegDebugHead) + 8;
    unsigned char* mem_start = (unsigned char*)malloc(real_mem_len);
    if (!mem_start) {
        return mem_start;
    }
    unsigned long ul = 0;
    memcpy(&ul, &mem_start, sizeof(mem_start));
    ul = ul%8;
    uint32_t offset = (ul == 0) ? 0:(8-ul);
    MemSegDebugHead* seg_head = (MemSegDebugHead*)(mem_start+offset);
    seg_head->offset = (uint8_t)offset;
    for (uint32_t i = 0; i < sizeof(seg_head->prob); ++i) {
        seg_head->prob[i] = MEM_DETECT_VAL;
    }
    GetLink()->Append(mem_start);
    return mem_start+offset+sizeof(MemSegDebugHead);
}

void GoogleMemPool::PutMemoryBytes(unsigned char* mem_start) {
    MemSegDebugHead* seg_head = (MemSegDebugHead*)(mem_start - sizeof(MemSegDebugHead));
    for (uint32_t i = 0; i < sizeof(seg_head->prob); ++i) {
        if(seg_head->prob[i] != MEM_DETECT_VAL) {
            LOG(FATAL) << "invalid segment state.maybe memory overflow.";
            return;
        }
    }
    unsigned char*  seg_start = mem_start - sizeof(MemSegDebugHead) - seg_head->offset;
    if(!GetLink()->Delete(seg_start)) {
        LOG(FATAL) << "invalid segment state. maybe memory overflow.";
        return;
    }
    free(seg_start);
}
#else
// warning C4100: "uLine": 未引用的形参
#ifdef WIN32
#pragma   warning(disable:4100)
#endif // WIN32

// 在进行内存垃圾回收过程中，会扫描已经分配的所有内存列表，
// 如果该列表所有内存都已经释放，那么该列表的空闲次数加1，
// 当空闲次数达到MEMPOOL_GARBAGE_COLLECTION_LOOP的时候，
// 该列表的内存块真正释放返还操作系统
#define MEMPOOL_GARBAGE_COLLECTION_LOOP 2

// 当空闲空闲内存/以分配内存>MEMPOOL_GARBAGE_COLLECTION_THRESHOLD的时候，
// 进行垃圾回收
#define MEMPOOL_GARBAGE_COLLECTION_THRESHOLD 0.7

// 如果用户没有设定最大内存，那么在内存池已经分配内存
// 大于该值的时候，启动内存垃圾回收
#define MEMPOOL_DEFAULT_LIMIT_SIZE 1024*1024*1024*1

// 只有当已分配内存大于该值才会启动内存垃圾回收
#define MEMPOOL_GARBAGE_COLLECTION_LOW_LEVEL 1024 * 1024 * 512

//
// 将内存收整,
// 尺寸为MEMPOOL_ROUND_SIZE的最小整倍数
// **修改该值的同时修改下面g_MemPool_BlockSizeList数组,
// 保证最后一个比MEMPOOL_ROUND_SIZE大**
//
#define MEMPOOL_ROUND_SIZE(bytes_per_node)  ((bytes_per_node> 1024) ? \
    ((bytes_per_node > 1024*128) ? 4*1024 :1024) : 64)

//
// New MemoryPool
// 定义内存块大小级别
// 1:申请的内存大小m_size如果不大于这个列表范围，
//   则挑选第一个大于等于m_size的内存组
//
// 2:如果申请的内存大小大于该列表范围,
//   则尾部收整到最近的一个(N*MEMPOOL_ROUND_SIZE)后插入新内存组
//
// 3:空闲时所有内存块通过链表组织起来(指针复用存储空间,
//      _DEBUG模式下多一byte记录是否空闲状态,
//      m64 sizeof(ptr) = 8bytes,
//      所以最小级别从16(len+state(3bytes padding)+ptrNext)开始,
//      不能小于16, 指针空间存储数据时候复用)
//
// 4:请按从小到大的顺序修改, 初始化的时候靠这个增序, 没有排序操作
//
// 5:所有分配出来的内存开始地址是8的倍数, 所以归一取整的大小也按8的倍数取.
//
const uint32_t g_MemPool_SegSizeList[] = {
    16,
    32,
    64,
    128
};

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif //

//
// 大块内存, Chunk
// 大块内存会被分割为小节点片使用(一个Chunk会被分割为众多的segments)
//
struct MEMP_MemChunk {
    //  指向下一个mem chunk
    MEMP_MemChunk*  next;

    //
    //  检测内存泄露时候才会用到, 平时没啥用
    //
    //  本段Chunk包含有多少个Segments
    uint32_t        segs_count;

    //  从本MemChunk之后到第一个Segment的偏移是多少
    //  (保证开始位置内存地址是8的倍数)
    uint32_t        start_offset;

    MEMP_MemChunk() {
        next = 0;
        segs_count = 0;
        start_offset = 0;
    }
};

//
//  定义额外头部
//  每块数据node需要知道自己有效大小
//  Head of segment
//  保证m32, m64下结构大小为8x
//
struct MEMP_ExternalHead {
    MEMP_ExternalHead*      next;           // 指向下一个segment

    typedef struct _tagState {
        unsigned char   head_padding[2];    // 承担头部越界检测任务
        unsigned char   user_dat;           // padding, 填充数据中
        // 分出1 byte做其它临时用途

        unsigned char   state_val;          // MEMP_MemNodeState_FREE
        // or MEMP_MemNodeState_INUSE
    } _State;

    typedef union {
        unsigned long   _ul;                // --m32, m64下都能保证
        // sizeof(SegmentState) +
        // sizeof(next) = 8x
        _State          state;              // segment state
    } SegmentState;

    SegmentState        seg_state;

    uint32_t            mem_size;           //  max storage length of segment
    uint32_t            user_request_len;   //  用户申请的长度


    //
    // _DEBU时候辅助参数
    // 用于检测内存泄露
    // filename只存文件名, 不保存路径
    //
#if defined(_DEBUG)
    uint32_t        line;
    unsigned char   filename[36];
#endif // _DEBUG


    MEMP_ExternalHead() {
        //
        // init. seg_state
        //
        memset(&seg_state, 0, sizeof(seg_state));

        //
        // 填充值
        //
        seg_state._ul = 0xffffffff;

        // 用来填充的空间, 只有后面检测越界的时候才会用到
        seg_state.state.head_padding[0] = MEM_DETECT_VAL;
        next = 0;
        seg_state.state.state_val = MEMP_SegmentState_FREE;
        mem_size = 0;
        user_request_len = 0;

#if defined(_DEBUG)
        line = 0;
        memset(filename, 0, sizeof(filename));
#endif // _DEBUG
    }
};

//
// 内存块数组单元
//
struct MEMP_MemItem {
    MEMP_MemChunk*                  chunk_list_head;
    uint32_t                        seg_size;   // 每小片内存节点大小
    CXThreadMutex                   seg_mutex;  // 用于分配空闲节点互斥
    MemDualLink<MEMP_ExternalHead>  free_segs;  // 空闲内存节点
    uint8_t                         free_loop;  // 垃圾回收发现该内存块全部空闲的次数
    uint32_t                        seg_count;  // 总内存块个数
    MEMP_MemItem() {
        chunk_list_head = 0;
        seg_size = 0;
        free_loop = 0;
        seg_count = 0;
    }
// 因为free_segs里面还有mutex，所以不允许复制，赋值操作
private:
    MEMP_MemItem(const MEMP_MemItem&) {}
    MEMP_MemItem& operator=(const MEMP_MemItem&) {
        return *this;
    }
};

//
//
//  ___________________________________________________________________________
//     一大段连续的内存(chunk)被下面这种分割方式各割成众多的segment
//     (真正的数据存储节点)
//
//     所有空闲节点通过MEMP_MemItem结构形成链表
//
// |-chunk head-|-offset-|-external head-|-Mem size-|mem padding|next segment|
//                       |____________segment___________________|
//
//



#if defined(_DEBUG)
// 这个结构的所有bytes都会被填充为MEM_DETECT_VAL
struct MEMP_Padding {
    unsigned char  padding[8];
    MEMP_Padding() {
        memset(padding, MEM_DETECT_VAL, sizeof(padding));
    }
};
#define SIZE_OF_PADDING sizeof(MEMP_Padding)
#else // RELEASE
#define SIZE_OF_PADDING 0
#endif //

#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif //

CMemPoolObj::~CMemPoolObj() {
#ifdef _DEBUG
    LOG(INFO) << "~LiteMemPool(), Object destroy.\r\n";
#endif // _DEBUG

    Clean();
}

CMemPoolObj::CMemPoolObj() {
#ifdef _DEBUG

    const char* mem_leak_state = "turn on";
    LOG(INFO) << "LiteMemPool(), Object init. debug memory leak is:" << mem_leak_state;
#endif // _DEBUG

    m_mempool_limit_size = 0;
    m_alloced_mem_count = 0;
    m_freed_mem_count = 0;

    // 内存数组
    m_mem_items = 0;
    m_items_array_len = 0;
    m_valid_items = 0;

    // 初始化
    uint32_t num_seg_count = sizeof(g_MemPool_SegSizeList) / sizeof(uint32_t);
    uint32_t arr_len = num_seg_count < 8 ? 8:num_seg_count;

    m_mem_items = new MEMP_MemItem*[arr_len];
    if (m_mem_items) {
        //  为每个内存块初始化
        for (uint32_t u = 0; u < num_seg_count; u++) {
            m_mem_items[u] = new MEMP_MemItem;
            m_mem_items[u]->seg_size = g_MemPool_SegSizeList[u];
        }
        m_items_array_len = arr_len;
        m_valid_items = num_seg_count;
    }
}

//
// 在Clean的时候可以设置该回调函数,用于传回泄漏的地址
//
MEMLEAK_CALLBACK g_memleak_callback = NULL;
void SetMemLeakCallback(MEMLEAK_CALLBACK cb){
    g_memleak_callback = cb;
}


void CMemPoolObj::FreeChunk(MEMP_MemChunk* chunk, uint32_t seg_size) {
    //  检测内存泄露
#if defined(_DEBUG)
    if (chunk->segs_count) {
        unsigned char* ptr = (unsigned char*)chunk +
                             sizeof(MEMP_MemChunk) +
                             chunk->start_offset;
        for (uint32_t j = 0; j < chunk->segs_count; j++) {
            MEMP_ExternalHead* seg =
                reinterpret_cast<MEMP_ExternalHead*>(ptr);

            // ? Memory leak
            if (seg->seg_state.state.state_val ==
                    MEMP_SegmentState_INUSE) {
                        LOG(ERROR) << "Detected memory leak in file:"
                           << reinterpret_cast<const char*>(seg->filename) << ", at line:"
                           << seg->line << " ,leak addr:"
                           << reinterpret_cast<void*>(ptr + sizeof(MEMP_ExternalHead));

                        // 如果用户设置有回调函数则回调一次
                        if(g_memleak_callback){
                            g_memleak_callback(
                              reinterpret_cast<void*>(ptr + sizeof(MEMP_ExternalHead)));
						}

                //
                // 故意不释放p, 可以让上层WIN32内存检测工具探测到。
                // 如果上层内存泄露工具探测到这里,
                // 具体内存泄露信息记录在上一代码行的szLeakInfo中.
                //
                //
#ifdef WIN32
                // 暂时关闭让win32内存检测工具检测内存泄漏
                // char* p = new char;
                // p = 0;
#endif //
            }

            ptr += (sizeof(MEMP_ExternalHead) +
                    seg_size +
                    SIZE_OF_PADDING);
        }
    }
#endif // _DEBUG

    delete []((unsigned char*)chunk);
}

void CMemPoolObj::Clean() {
    CXThreadAutoLock autoLock(&m_mutex);

    //  释放内存
    if (m_mem_items) {
        for (uint32_t u = 0; u < m_valid_items; u++) {
            MEMP_MemChunk* chunk_head = m_mem_items[u]->chunk_list_head;
            while (chunk_head) {
                MEMP_MemChunk* chunk = chunk_head;
                chunk_head = chunk_head->next;
                FreeChunk(chunk,m_mem_items[u]->seg_size);
                chunk = 0;
            }
            //m_mem_items[u].chunk_list_head = 0;
            delete m_mem_items[u];
            m_mem_items[u] = 0;
        }
        delete []m_mem_items;
        m_mem_items = 0;
    }

    m_items_array_len = m_valid_items = 0;
}

void CMemPoolObj::Debug() {
    for (uint32_t u = 0; u < m_valid_items; u++) {
        if (u > 0 &&
                m_mem_items[u]->seg_size <= m_mem_items[u-1]->seg_size) {
            ABORT;
        }
    }
}

void CMemPoolObj::SetMemLimitSize(unsigned long uMaxMemSize) {
    CXThreadAutoLock autoLock(&m_mutex);
    m_mempool_limit_size = uMaxMemSize;
}

unsigned char* CMemPoolObj::GetMemory(uint32_t mem_len,
                                      const char* file_name,
                                      uint32_t line_num) {

    unsigned char* ptr = GetMemory(mem_len);

#ifdef _DEBUG
    if (ptr) {
        MEMP_ExternalHead* seg_head =
            reinterpret_cast<MEMP_ExternalHead*>(
                ptr - sizeof(MEMP_ExternalHead)
            );

        // line number
        seg_head->line = line_num;

        // 保存文件名，防止越界
        // _DEBUG模式file_name必须存在
        CHECK(file_name);
        const char* name_start = strrchr(file_name, '/');

        if (!name_start)
            name_start = strrchr(file_name, '\\');
        if (name_start)
            name_start++;
        else
            name_start = file_name;

        uint32_t len = STRLEN(name_start);
        len = (len >= sizeof(seg_head->filename)) ?
              sizeof(seg_head->filename) - 1 :
              len;

        memcpy(seg_head->filename, name_start, len);
        seg_head->filename[len] = 0;

        if (seg_head->line == 0) {
            LOG(FATAL) << "error,line_num = " << line_num;
        }
    }
#endif

    return ptr;
}

//
//  获取指定长的内存块(有可能返回的内存大于等于要求的内存长度)
//
unsigned char* CMemPoolObj::GetMemory(uint32_t request_mem_len) {
    assert(request_mem_len != 0);

    //CXThreadAutoLock autoLock(&m_mutex);
    unsigned char* ptr_mem = 0;

    //
    // 处理过大, 不需要cache的内存块
    // 大内存块格式
    //  offset(x)   ExternalHead
    // |----------|---------------|------mem size-----|-mem padding-|
    //
    if (request_mem_len >= MEMPOOL_MAX_CACHE_SIZE) {
        uint32_t total_mem_len = 8 +
                                 sizeof(MEMP_ExternalHead) +
                                 request_mem_len +
                                 SIZE_OF_PADDING;
        if (m_mempool_limit_size &&
                (m_alloced_mem_count+total_mem_len > m_mempool_limit_size)) {
            //  检查是否超出内存限制
            LOG(ERROR) << "out of memory limit."
                       "user request mem size:" << request_mem_len << " bytes\r\n";
            return 0;
        }

        unsigned char* p = new unsigned char[total_mem_len];
        if (!p) {
            LOG(ERROR) << "***ERROR, Level 3***, out of memory";
        }

        // cal. offset
        unsigned char* ptr = p+sizeof(MEMP_ExternalHead);
        unsigned long ul = 0;
        void* void_ptr = NULL;
        memcpy(&ul, &ptr, sizeof(void_ptr));
        ul = ul%8;
        uint32_t offset = (ul == 0) ? 0:(8-ul);

        //                p+sizeof(MEMP_ExternalHead)+offset
        unsigned char* mem_start = p+offset+sizeof(MEMP_ExternalHead);
        MEMP_ExternalHead* head =
            reinterpret_cast<MEMP_ExternalHead*>(p + offset);

        head->next = 0;
        head->mem_size = request_mem_len;
        head->user_request_len = request_mem_len;
        head->seg_state.state.state_val = MEMP_SegmentState_INUSE;
        head->seg_state.state.user_dat = (unsigned char)offset;
#if defined(_DEBUG)
        memset(head->seg_state.state.head_padding,
               MEM_DETECT_VAL,
               sizeof(head->seg_state.state.head_padding));

        memset(mem_start + request_mem_len,
               MEM_DETECT_VAL,
               SIZE_OF_PADDING);
#endif // _DEBUG

        return mem_start;
    }

    //
    // 查找是否在某块内存上已经存在适合该尺寸的内存块集合, 如果不存在则自动插入
    // 如果查找不到合适的位置则将uMemLen对齐到MEMPOOL_ROUND_SIZE
    // 后插入一个新的内存集合
    //
    MEMP_MemItem* mem_item = NULL;
    if (!FindMemItemPos(request_mem_len, &mem_item, true)) {
#ifdef _DEBUG
        // 一定是可以找到的。
        LOG(FATAL) << "Find memory item pos fail, "
                   "request mem len:" << request_mem_len;
#endif //
    }

    // 清空内存列表被检测为空闲的次数    
    {
        CXThreadAutoLock autoLock(&(mem_item->seg_mutex));
        mem_item->free_loop = 0;    
    }


    // 检查内存是否足够, 内存不够时新分配一批内存块,
    // 同时检测是否超出总内存大小限制
    //
    // MEMP_MemChunk offset(x)|-ExternalHead-segsize(storage)-MemPadding-|
    // |------------|---------|-------------|----------------|-----------|
    //

    MEMP_ExternalHead* seg = mem_item->free_segs.Front();
    do {
        if(seg)
            break;
        // 多个线程可能同时检测到现在没有可用节点
        CXThreadAutoLock autoLock(&(mem_item->seg_mutex));
        seg = mem_item->free_segs.Front();
        if(seg)
            break;
        //
        //  计算需要分配多长的连续内存= blocks *
        //                              (external_head + node_size +
        //                               SIZE_OF_PADDING)
        // 每个节点长度
        //
        unsigned long per_seg_len= sizeof(MEMP_ExternalHead) +
                                   mem_item->seg_size +
                                   SIZE_OF_PADDING;

        // 一共多少个节点
        unsigned long segs = MAX_MEMP_NODES(mem_item->seg_size);

        // 总长度 +8(保证每个seg开始地址是8的倍数)
        unsigned long chunk_len = sizeof(MEMP_MemChunk)+8+segs*per_seg_len;

        //  检查内存是否超限定
        if (m_mempool_limit_size>0) {
            //  内存超限制大小
            if (m_alloced_mem_count+chunk_len > m_mempool_limit_size) {
                if (m_mempool_limit_size > m_alloced_mem_count) {
                    // Use limit blocks
                    segs = (m_mempool_limit_size-m_alloced_mem_count) /
                           per_seg_len;
                    // Update chunk_len
                    chunk_len = sizeof(MEMP_MemChunk)+8+segs*per_seg_len;
                    if (segs == 0) {
                        // 超出内存限制
                        LOG(ERROR) << "out of memory limit.";
                        return 0;
                    }
                } else {
                    //  Out of memory
                    LOG(ERROR) << "out of memory limit."
                               "max mem limit to:" <<
                               m_mempool_limit_size <<
                               " bytes, now alloced:" << m_alloced_mem_count << " bytes";
                    return 0;
                }
            }
        }

        //  新分配一大块内存块
        unsigned char* bytes = new unsigned char[chunk_len];
        if (!bytes) {
            LOG(ERROR) << "out of memory.try new data fail.";
            return 0;
        }

        // 更新内存块列表总的块数
        mem_item->seg_count += segs;

        //  Update alloced length
        m_alloced_mem_count += chunk_len;

        // 更新空闲内存大小
        m_freed_mem_count += chunk_len;

        // Add MemChunk to ChunkList of MemItem
        MEMP_MemChunk* chunk = reinterpret_cast<MEMP_MemChunk*>(bytes);
        chunk->next = mem_item->chunk_list_head;
        mem_item->chunk_list_head = chunk;

        // Add to free list
        // 强行让每个节点开始地址为8的倍数
        unsigned char* seg_start = bytes + sizeof(MEMP_MemChunk);
        unsigned long ul = 0;

        void* void_ptr = NULL;
        memcpy(&ul, &seg_start, sizeof(void_ptr));
        ul = ul % 8;
        // update segs and offset
        chunk->start_offset = (ul == 0) ? 0:(8-ul);
        chunk->segs_count = segs;
        seg_start += chunk->start_offset;
        MEMP_ExternalHead* seg_item =
            reinterpret_cast<MEMP_ExternalHead*>(seg_start);

        for (uint32_t j = 0; j < segs; j++) {
            seg_item->next = 0;
            seg_item->seg_state.state.state_val = MEMP_SegmentState_FREE;
            seg_item->mem_size = mem_item->seg_size;
            seg_item->user_request_len = 0;
#if defined(_DEBUG)
            seg_item->line = 0;
            seg_item->filename[0] = 0;
            memset(seg_item->seg_state.state.head_padding,
                   MEM_DETECT_VAL,
                   sizeof(seg_item->seg_state.state.head_padding));
#endif // _DEBUG

            unsigned char* next_pos = reinterpret_cast<unsigned char*>(seg_item)
                                      + per_seg_len;
            // add to free list
            mem_item->free_segs.Append(seg_item);

            // seek to next segment
            seg_item = reinterpret_cast<MEMP_ExternalHead*>(next_pos);
        }
        seg = mem_item->free_segs.Front();
    } while(0);

    //  分配一个内存块给外部使用
    if (seg) {
        // 更新空闲内存大小
        m_freed_mem_count -= seg->mem_size;

        seg->user_request_len = request_mem_len;

#ifdef _DEBUG
        // Check len.
        if (seg->mem_size != mem_item->seg_size) {
            // Error
            LOG(FATAL) << "maybe memory overflow.";
        }
        seg->seg_state.state.state_val = MEMP_SegmentState_INUSE;

        //
        // 初始化memory overflow padding area, 用于检测内存越界
        //
        //  request_mem_len:user request length
        unsigned char* mem_overflow_padding = (unsigned char*)seg +
                                              sizeof(MEMP_ExternalHead) +
                                              request_mem_len;

        memset(mem_overflow_padding, MEM_DETECT_VAL, SIZE_OF_PADDING);
#endif // _DEBUG
        //  给外部存储使用的时候跳过头部私有数据区，next部分复用存储
        ptr_mem = ((unsigned char*)seg)+sizeof(MEMP_ExternalHead);
    }

    return ptr_mem;
}

//
// 将内存放回内存池
//
void CMemPoolObj::PutMemoryBytes(unsigned char* &ptr_mem) {
    if (!ptr_mem)
        return;

    //CXThreadAutoLock autoLock(&m_mutex);
    MEMP_ExternalHead* seg =
        reinterpret_cast<MEMP_ExternalHead*>((unsigned char*)ptr_mem -
                sizeof(MEMP_ExternalHead));

    // 更新空闲内存大小
    m_freed_mem_count += seg->mem_size;

    // check segment state and memory overflow
#ifdef _DEBUG
    if (seg->seg_state.state.state_val != MEMP_SegmentState_INUSE) {
        //  Error
        LOG(FATAL) << "invalid segment state. maybe memory overflow.";
        ptr_mem = 0;
        return;
    }

    MEMP_Padding padding;

    //
    // 检查是否头尾被压
    //
    if (seg->seg_state.state.head_padding[0] != MEM_DETECT_VAL ||
            memcmp(ptr_mem+seg->user_request_len, &padding, sizeof(padding)) != 0) {
        // Error, 内存越界
        LOG(FATAL) << "Detected memory overflow in file: " <<
                   reinterpret_cast<char*>(seg->filename) << ", at line:" << seg->line;
        ptr_mem = 0;
        return;
    }

    //
    //  清空上次mempool_NEW位置信息
    //
    seg->line = 0;
    seg->filename[0] = 0;
#endif //

    //
    // 处理过大, 不需要cache的内存块
    // 大内存块格式
    //  offset(x)   ExternalHead
    // |----------|---------------|------mem size-----|-mem padding-|
    //
    if (seg->user_request_len >= MEMPOOL_MAX_CACHE_SIZE) {
        unsigned char* bytes= ptr_mem -
                              sizeof(MEMP_ExternalHead) -
                              seg->seg_state.state.user_dat;
        delete []bytes;
        bytes = 0;
        ptr_mem = 0;
        return;
    }

    //
    // in cache segment
    // Find pos
    //
    MEMP_MemItem* mem_item;
    if (!FindMemItemPos(seg->mem_size, &mem_item, false)) {
        //  Error, 内存被压
        LOG(FATAL) << "maybe memory overflow.";
        ptr_mem = 0;
        return;
    }

    //  Add to free list
#ifdef _DEBUG
    seg->seg_state.state.state_val = MEMP_SegmentState_FREE;
#endif //
    mem_item->free_segs.Append(seg);
    ptr_mem = 0;

    // 如果已经分配的内存大于内存限制的2/3，启动内存垃圾回收
    if (m_mempool_limit_size > 0 && GetMemPoolSize() > m_mempool_limit_size * 2 / 3) {
        GarbageCollection();
    } else if ((double)m_freed_mem_count / (double)m_alloced_mem_count >
               MEMPOOL_GARBAGE_COLLECTION_THRESHOLD &&
               m_alloced_mem_count > MEMPOOL_GARBAGE_COLLECTION_LOW_LEVEL) {

        GarbageCollection();
    }
}

uint32_t CMemPoolObj::GetMemPoolSize() const {
    return m_alloced_mem_count;
}

void CMemPoolObj::GarbageCollection() {
    m_rwlock.WriterLock();
    for(uint32_t i = 0; i != m_valid_items; ++i) {
        MEMP_MemItem* tmp = m_mem_items[i];
        CXThreadAutoLock autoLock(&(tmp->seg_mutex));
        if(!tmp->free_segs.Empty() &&
                tmp->seg_count == tmp->free_segs.Size()) {
            ++tmp->free_loop;
            if (tmp->free_loop == MEMPOOL_GARBAGE_COLLECTION_LOOP) {
                m_alloced_mem_count -= tmp->seg_count * tmp->seg_size;
                m_freed_mem_count -= tmp->seg_count * tmp->seg_size;

                MEMP_MemChunk* chunk_head = tmp->chunk_list_head;
                while (chunk_head) {
                    MEMP_MemChunk* chunk = chunk_head;
                    chunk_head = chunk_head->next;
                    FreeChunk(chunk,tmp->seg_size);
                    chunk = 0;
                }
                tmp->chunk_list_head = NULL;
                tmp->free_segs.Clear();
                tmp->free_loop = 0;
                tmp->seg_count = 0;
            }
        }
    }
    m_rwlock.Unlock();
}
//
//  1:查找第一次出现大于等于MemSize的位置(MemSize是要求要分配的大小)
//  2:bAutoInsert:如果不存在是否自动插入
//
bool CMemPoolObj::FindMemItemPos(uint32_t mem_size,  MEMP_MemItem** mem_item,
                                 bool auto_insert) {
    if (auto_insert) {
        m_rwlock.WriterLock();
    } else {
        m_rwlock.ReaderLock();
    }

    bool ret = false;
    uint32_t insert_pos = 0;
    int32_t find_pos = -1;

    //  比最小尺寸还小
    if (mem_size <= m_mem_items[0]->seg_size) {
        *mem_item = m_mem_items[0];
        ret = true;
    } else if (mem_size > m_mem_items[m_valid_items-1]->seg_size) { // 大于最大内存
        if (auto_insert) {
            insert_pos = m_valid_items;
            ret = true;
        } else {
            ret = false;
        }
    } else if(auto_insert) { // 处于最小和最大之间,可以进行插入操作,由GetMemeroy调用
        // 进行二分查找
        int32_t left = 0;
        int32_t right = m_valid_items-1;
        int32_t mid = 0;
        while (left <= right) {
            mid = (left+right)/2;
            if (mem_size >= m_mem_items[mid]->seg_size &&
                    mem_size <= m_mem_items[mid+1]->seg_size) {
                // Find it
                ret = true;
                if (mem_size == m_mem_items[mid]->seg_size) {
                    *mem_item = m_mem_items[mid];
                    find_pos = mid;
                } else if (mem_size == m_mem_items[mid+1]->seg_size) {
                    *mem_item = m_mem_items[mid+1];
                    find_pos = mid + 1;
                } else { //  必然处于两者之间
                    if ( (mid+1) <
                            int32_t(sizeof(g_MemPool_SegSizeList) /
                                    sizeof(uint32_t)) ) {
                        *mem_item = m_mem_items[mid+1]; // 这种情况不需要新插入
                        find_pos = mid + 1;
                    } else { //  在初始化列表之外
                        // 要开辟的内存大小收整到MEMPOOL_ROUND_SIZE以后
                        // 是否等于右边的内存大小
                        if (FIXED_NODE_SIZE_TO_ROUND_SIZE_N(mem_size)  ==
                                m_mem_items[mid+1]->seg_size) {
                            // 这种情况也不需要新插入,直接使用右边一个内存块
                            *mem_item = m_mem_items[mid+1];
                            find_pos = mid + 1;
                        } else
                            insert_pos = mid+1;
                    }
                }
                break;
            }

            if (m_mem_items[mid]->seg_size > mem_size)
                right = mid-1;
            else
                left = mid+1;
        }
    } else { // 处于最小和最大之间,并且只是进行查询，由PutMemeroy调用
        // 进行二分查找
        int32_t left = 0;
        int32_t right = m_valid_items-1;
        int32_t mid = 0;
        while (left <= right) {
            mid = (left+right)/2;
            if (mem_size >= m_mem_items[mid]->seg_size &&
                    mem_size <= m_mem_items[mid+1]->seg_size) {
                // Find it
                ret = true;
                if (mem_size == m_mem_items[mid]->seg_size) {
                    *mem_item = m_mem_items[mid];

                } else if (mem_size == m_mem_items[mid+1]->seg_size) {
                    *mem_item = m_mem_items[mid+1];
                } else { //  必然处于两者之间
                    // 如果不是auto insert则必须找到大小一致的节点
                    ret = false;
                }
                break;
            }

            if (m_mem_items[mid]->seg_size > mem_size)
                right = mid-1;
            else
                left = mid+1;
        }
    }

    if (!auto_insert) {
        m_rwlock.Unlock();
        return ret;
    }

    uint32_t pos = find_pos;
    if (insert_pos > 0) {
        pos = insert_pos;
    }
    // 搜索更大空闲内存块列表
    while (pos < m_valid_items && m_mem_items[pos]->seg_size <= MAX_HOP_SIZE(mem_size)) {
        if (!m_mem_items[pos]->free_segs.Empty()) {
            *mem_item = m_mem_items[pos];
            m_rwlock.Unlock();
            return true;
        }
        ++pos;
    }
    // auto_insert is true
    // 处理新插入节点, auto_insert = true
    if (insert_pos > 0) { // 需要插入节点
        ret = false;

        // insert node
        // 需要扩大数组, 插入新的内存块集合
        if (m_valid_items >= m_items_array_len) {
            MEMP_MemItem** items = new MEMP_MemItem*[m_items_array_len+256];
            if (items) {
                for (uint32_t i = 0; i != m_valid_items; i++) {
                    items[i] = m_mem_items[i];
                }

                delete []m_mem_items;
                m_mem_items = items;
                m_items_array_len = m_items_array_len+256;
            }
        }

        // Insert
        if (insert_pos < m_items_array_len) {
            // If not the last one
            if (insert_pos < m_valid_items) {
                for (uint32_t u = m_valid_items; u > insert_pos; u--) {
                    m_mem_items[u] = m_mem_items[u-1];
                }
            }

            m_mem_items[insert_pos] = new MEMP_MemItem;
            //  向上对齐到MEMPOOL_ROUND_SIZE
            m_mem_items[insert_pos]->seg_size =
                FIXED_NODE_SIZE_TO_ROUND_SIZE_N(mem_size);

            *mem_item = m_mem_items[insert_pos];
            m_valid_items++;
            ret = true;
        }
    }
    m_rwlock.Unlock();
    return ret;
}

// ----------------------------------------------------------------------------

void mempool_debug_is_valid_ptr(const char* file, const int32_t line,
                                unsigned char* ptr_mem) {
#ifdef _DEBUG
    if (ptr_mem) {
        MEMP_ExternalHead* seg =
            reinterpret_cast<MEMP_ExternalHead*>(
                ptr_mem - sizeof(MEMP_ExternalHead)
            );

        if (!(seg->seg_state.state.state_val == MEMP_SegmentState_INUSE
                || seg->seg_state.state.state_val == MEMP_SegmentState_FREE)) {
            //  Error, invalid state, maybe memory overflow
            LOG(FATAL) << file << ":" << line << ", Invalid ptr_mem";
            return;
        }
    }
#else
// nothing for release version
if (!ptr_mem) {
    LOG(ERROR) << file << ":" << line << ", Invalid ptr_mem.";
}
#endif //
}

#endif // USING_GOOGLE_MEMPOOL
_END_XFS_BASE_NAMESPACE_
