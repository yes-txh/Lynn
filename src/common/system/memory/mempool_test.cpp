#include "common/system/memory/mempool.hpp"
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <gtest/gtest.h>
#include "common/base/compatible/malloc.h"

class MemPoolTest: public testing::Test
{
    virtual void SetUp()
    {
        MemPool_Initialize();
    }

    virtual void TearDown()
    {
        MemPool_Destroy();
    }
};

TEST_F(MemPoolTest, SmallSize)
{
    for (int n = 0; n < 1024; ++n)
    {
        for (size_t size = 1; size < 4096; size += 3)
        {
            unsigned char* p = static_cast<unsigned char*>(MemPool_Allocate(size));
            size_t block_size = MemPool_GetBlockSize(p);
            p[0] = 0xCF;
            p[size - 1] = 0xCF;
            EXPECT_GE(block_size, size);
            if (size > 1024)
                EXPECT_LE(block_size, 2 * size);
            MemPool_Free(p);
        }
    }
}

TEST_F(MemPoolTest, SmallSizeMalloc)
{
    for (int n = 0; n < 1024; ++n)
    {
        for (size_t size = 1; size < 4096; size += 3)
        {
            unsigned char* p = static_cast<unsigned char*>(malloc(size));
            size_t block_size = malloced_size(p);
            p[0] = 0xCF;
            p[size - 1] = 0xCF;
            EXPECT_GE(block_size, size);
            free(p);
        }
    }
}

TEST_F(MemPoolTest, LargeSize)
{
    for (size_t size = 1024; size < MAX_MEMUNIT_SIZE; size += 4096)
    {
        unsigned char* p = static_cast<unsigned char*>(MemPool_Allocate(size));
        size_t block_size = MemPool_GetBlockSize(p);
        EXPECT_GE(block_size, size);
        if (size > 1024)
            EXPECT_LE(block_size, 2 * size);
        p[0] = 0xCF;
        p[size - 1] = 0xCF;
        MemPool_Free(p);
    }
}

TEST_F(MemPoolTest, HugeSize)
{
    for (int i = 0; i < 100; ++i)
    {
        size_t size = (i + 32) * 1024  * 1024;
        unsigned char* p = static_cast<unsigned char*>(MemPool_Allocate(size));
        ASSERT_TRUE(p != NULL);
        MemPool_Free(p);
    }
}

TEST_F(MemPoolTest, LargeSizeMalloc)
{
    for (size_t size = 1024; size < MAX_MEMUNIT_SIZE; size += 4096)
    {
        unsigned char* p = static_cast<unsigned char*>(malloc(size));
        size_t block_size = malloced_size(p);
        EXPECT_GE(block_size, size);
        p[0] = 0xCF;
        p[size - 1] = 0xCF;
        free(p);
    }
}

TEST_F(MemPoolTest, GetAllocatedSizeAndGetPooledSize)
{
    EXPECT_EQ(0U, MemPool_GetAllocatedSize());
    EXPECT_EQ(0U, MemPool_GetPooledSize());

    unsigned char* p = static_cast<unsigned char*>(MemPool_Allocate(1024));
    size_t block_size = MemPool_GetBlockSize(p);
    EXPECT_EQ(block_size, MemPool_GetAllocatedSize());
    EXPECT_EQ(0U, MemPool_GetPooledSize());

    MemPool_Free(p);
    EXPECT_EQ(0U, MemPool_GetAllocatedSize());
    EXPECT_EQ(block_size, MemPool_GetPooledSize());
}


