#include "common/system/memory/mempool.hpp"
#include "common/system/concurrency/atomic/atomic.h"
#include "common/base/array_size.h"
#include <assert.h>
#include <algorithm>

struct MemPool::BLOCK_HEADER
{
    unsigned char Magic[2]; // magic number to check, must be 'MP'
    short Index;            // 内存单元索引，-1 表示不在 cache 里
    unsigned int Size;      // 单元的大小
};


static const size_t  s_wSizeArray[] =
{
    MIN_MEMUNIT_SIZE,
    2 * MIN_MEMUNIT_SIZE,
    4 * MIN_MEMUNIT_SIZE,
    8 * MIN_MEMUNIT_SIZE,
    16 * MIN_MEMUNIT_SIZE,
    32 * MIN_MEMUNIT_SIZE,
    64 * MIN_MEMUNIT_SIZE,
    128 * MIN_MEMUNIT_SIZE,
    256 * MIN_MEMUNIT_SIZE,
    512 * MIN_MEMUNIT_SIZE,
    1024 * MIN_MEMUNIT_SIZE,
    2048 * MIN_MEMUNIT_SIZE,
    2 * 2048 * MIN_MEMUNIT_SIZE,
    4 * 2048 * MIN_MEMUNIT_SIZE,
    8 * 2048 * MIN_MEMUNIT_SIZE,
    16 * 2048 * MIN_MEMUNIT_SIZE
};

static const size_t s_wDefaultInitialCount[] =
{
    50, // 1024,
    50, // 2048,
    50, // 1024,
    50, // 2048,
    50, // 1024,
    50, // 1024,
    50, // 128,
    50,
    50,
    8,
    4,
    2,
    1,
    1,
    1,
    1
};

MemPool::MemPool(size_t dwMaxMemSize):
    m_poolMutex(),
    m_dwQueueNum(MAX_INDEX_NUM),
    m_allocated_memory_size(0),
    m_pooled_memory_size(0),
    m_dwAllocateCount(0),
    m_dwFreeCount(0),
    m_dwNewCount(0),
    m_dwDeleteCount(0)
{
    assert(dwMaxMemSize >= MIN_MEMUNIT_SIZE);
    assert((dwMaxMemSize % MIN_MEMUNIT_SIZE) == 0);
    assert(dwMaxMemSize <= MAX_MEMUNIT_SIZE);
}

MemPool::~MemPool()
{
    //////////////////////////////////////////////////////////////////////////
    // 释放MemPool中的内存;
    BLOCK_HEADER* pBlock = NULL;

    for (unsigned int i = 0; i < m_dwQueueNum; i++)
    {
        while (!(m_memPool[i].empty()))
        {
            pBlock = m_memPool[i].front();

            if (pBlock != NULL)
            {
                FreeBlock(pBlock);
            }

            m_memPool[i].pop_front();
        }
    }
}

bool MemPool::IsValidMemoryBlock(const MemPool::BLOCK_HEADER* block_header)
{
    if (block_header->Magic[0] != 'M' || block_header->Magic[1] != 'P')
        return false;

    if (block_header->Index == -1)
        return block_header->Size > MAX_MEMUNIT_SIZE;

    return block_header->Index < MAX_INDEX_NUM &&
        block_header->Size <= MAX_MEMUNIT_SIZE;
}

MemPool::BLOCK_HEADER* MemPool::AllocateBlock(unsigned int index)
{
    size_t size = sizeof(BLOCK_HEADER) + s_wSizeArray[index];
    BLOCK_HEADER* p = static_cast<BLOCK_HEADER*>(operator new(size));
    p->Magic[0] = 'M';
    p->Magic[1] = 'P';
    p->Index = index;
    p->Size = s_wSizeArray[index];
    AtomicIncrement(m_dwNewCount);
    return p;
}

MemPool::BLOCK_HEADER* MemPool::AllocateLargeBlock(size_t size)
{
    size_t new_size = sizeof(BLOCK_HEADER) + size;
    BLOCK_HEADER* p = static_cast<BLOCK_HEADER*>(operator new(new_size));
    p->Magic[0] = 'M';
    p->Magic[1] = 'P';
    p->Index = -1;
    p->Size = size;
    AtomicIncrement(m_dwNewCount);
    return p;
}

void MemPool::FreeBlock(BLOCK_HEADER* block)
{
    operator delete(block);
    AtomicDecrement(m_dwDeleteCount);
}

//////////////////////////////////////////////////////////////////////////
// 功能描述:             分配一个大小为dwSize的MemUnit
// 输入参数:             @参数1: dwSize; MemUnit的大小;
// 返回值:               MemUnit的指针;
void* MemPool::Allocate(size_t dwSize)
{
    BLOCK_HEADER* pBlock = NULL;
    if (dwSize != 0)
    {
        int dwArrayIndex = GetUnitIndex(dwSize);

        if (dwArrayIndex >= MAX_INDEX_NUM)
        {
            pBlock = AllocateLargeBlock(dwSize);
        }
        else
        {
            {
                Spinlock::Locker locker(&m_poolMutex);

                if (!m_memPool[dwArrayIndex].empty())
                {
                    m_pooled_memory_size -= s_wSizeArray[dwArrayIndex];
                    pBlock = m_memPool[dwArrayIndex].front();
                    m_memPool[dwArrayIndex].pop_front();
                }
            }

            if (!pBlock)
            {
                pBlock = AllocateBlock(dwArrayIndex);
            }
        }
    }

    if (pBlock)
    {
        Spinlock::Locker locker(&m_poolMutex);
        m_dwAllocateCount++;
        m_allocated_memory_size += pBlock->Size;
        return pBlock + 1;
    }

    return NULL;
}

// 功能描述:                释放一个MemUnit的资源
// 输入参数:                @参数1: pUnit, 内存块的指针;
// 返回值:              无
void MemPool::Free(void* p)
{
    if (p)
    {
        BLOCK_HEADER* pBlock = reinterpret_cast<BLOCK_HEADER*>(p) - 1;
        assert(IsValidMemoryBlock(pBlock));

        int dwArrayIndex = pBlock->Index;
        size_t size = pBlock->Size;

        if (dwArrayIndex == -1)
        {
            FreeBlock(pBlock);
        }
        else
        {
            {
                Spinlock::Locker locker(&m_poolMutex);
                size_t dwCurQueueSize = m_memPool[dwArrayIndex].size();
                if (dwCurQueueSize < s_wDefaultInitialCount[dwArrayIndex])
                {
                    m_memPool[dwArrayIndex].push_back(pBlock);
                    m_pooled_memory_size += s_wSizeArray[dwArrayIndex];
                    pBlock = NULL;
                }
            }

            if (pBlock)
            {
                FreeBlock(pBlock);
            }
        }

        AtomicIncrement(m_dwFreeCount);
        AtomicSub(m_allocated_memory_size, size);
    }
}

size_t MemPool::GetBlockSize(const void* ptr) const
{
    if (ptr)
    {
        const BLOCK_HEADER* pBlock = reinterpret_cast<const BLOCK_HEADER*>(ptr) - 1;
        return pBlock->Size;
    }
    return 0;
}

size_t MemPool::GetAllocatedSize() const
{
    return m_allocated_memory_size;
}

size_t MemPool::GetPooledSize() const
{
    return m_pooled_memory_size;
}

// 功能描述:                获取内存块的循环Buffer索引编号;
// 输入参数:                @参数1: dwSize, 内存块的大小;
// 返回值:              索引编号值;
int MemPool::GetUnitIndex(size_t dwSize)
{
    const size_t* p = std::lower_bound(s_wSizeArray, s_wSizeArray + ARRAY_SIZE(s_wSizeArray), dwSize);
    return p - s_wSizeArray;
}

MemPool* s_pMemPool = NULL;

/// 初始化MemPool
bool MemPool_Initialize(size_t dwMaxMemSize)
{
    if (s_pMemPool == NULL)
        s_pMemPool = new MemPool(dwMaxMemSize);

    return true;
}

/// 释放MemPool
void MemPool_Destroy()
{
    delete s_pMemPool;
    s_pMemPool = NULL;
}

/// 线程安全的分配内存资源
void* MemPool_Allocate(size_t dwSize)
{
    assert(s_pMemPool != NULL);
    return s_pMemPool->Allocate(dwSize);
}

void MemPool_Free(void* p)
{
    assert(s_pMemPool);
    s_pMemPool->Free(p);
}

size_t MemPool_GetAllocatedSize()
{
    return s_pMemPool->GetAllocatedSize();
}

size_t MemPool_GetPooledSize()
{
    return s_pMemPool->GetPooledSize();
}

size_t MemPool_GetBlockSize(const void* ptr)
{
    assert(s_pMemPool);
    return s_pMemPool->GetBlockSize(ptr);
}

