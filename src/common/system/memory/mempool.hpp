#ifndef COMMON_SYSTEM_MEMORY_MEMPOOL_H
#define COMMON_SYSTEM_MEMORY_MEMPOOL_H

#include <assert.h>
#include <deque>
#include "common/system/concurrency/spinlock.hpp"

const size_t MIN_MEMUNIT_SIZE = 1024;
const size_t MAX_MEMUNIT_SIZE = 16 * 2048 * MIN_MEMUNIT_SIZE;
const int MAX_INDEX_NUM = 16;

class MemPool
{
    struct BLOCK_HEADER;
public:
    explicit MemPool(size_t dwMaxMemSize);

    ~MemPool();

    // 功能描述:             分配一个大小为dwSize的MemUnit
    // 输入参数:             @参数1: dwSize; MemUnit的大小;
    // 返回值:               MemUnit的指针;
    void* Allocate(size_t dwSize);

    // 功能描述:             释放一个MemUnit的资源
    // 输入参数:             @参数1: pUnit, 内存块的指针;
    // 返回值:               无
    void Free(void* p);

    size_t GetBlockSize(const void* ptr) const;
    size_t GetAllocatedSize() const;
    size_t GetPooledSize() const;

private:
    BLOCK_HEADER* AllocateBlock(unsigned int index);
    BLOCK_HEADER* AllocateLargeBlock(size_t size);
    void FreeBlock(BLOCK_HEADER* block);
    bool IsValidMemoryBlock(const BLOCK_HEADER* block_header);
    int GetUnitIndex(size_t dwSize);
private:
    Spinlock                m_poolMutex;
    std::deque<BLOCK_HEADER*>   m_memPool[MAX_INDEX_NUM];
    size_t m_dwQueueNum;
    size_t m_allocated_memory_size;
    size_t m_pooled_memory_size;
    size_t m_dwAllocateCount;
    size_t m_dwFreeCount;
    size_t m_dwNewCount;
    size_t m_dwDeleteCount;
};

//////////////////////////////////////////////////////////////////////////
// 初始化MemPool;
bool MemPool_Initialize(size_t dwMaxMemSize = MAX_MEMUNIT_SIZE);

//////////////////////////////////////////////////////////////////////////
// 释放MemPool;
void MemPool_Destroy();

//////////////////////////////////////////////////////////////////////////
// 线程安全的分配内存资源
void* MemPool_Allocate(size_t dwSize);
void MemPool_Free(void* p);

size_t MemPool_GetAllocatedSize();
size_t MemPool_GetPooledSize();

size_t MemPool_GetBlockSize(const void* ptr);

#endif // COMMON_SYSTEM_MEMORY_MEMPOOL_H

