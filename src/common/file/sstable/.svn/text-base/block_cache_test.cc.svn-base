/*
* =====================================================================================
*
*       Filename:  sstable_writer_block_test.cc
*
*    Description:  test for sstable_writer_block.cc
*
*        Version:  1.0
*        Created:  01/04/2011 12:23:29 PM
*       Revision:  none
*       Compiler:  gcc
*
*         Author:  cyluo (xingLuo), 312733291@qq.com
*        Company:  Tencent, China
*
* =====================================================================================
*/
#include <sys/time.h>
#include <gtest/gtest.h>
#include <math.h>

#include "glog/logging.h"
#include "glog/raw_logging.h"
#include "common/base/byte_order.hpp"
#include "common/config/cflags.hpp"
#include "common/system/time/time_utils.hpp"

#include "common/file/sstable/sstable_writer_block.h"
#include "common/file/sstable/sstable_reader_block.h"
#include "common/file/sstable/block_cache.h"
#include "common/file/sstable/test_helper.h"

uint32_t g_url_num = 100;                   // 从文件中读取的url的数目
uint32_t g_record_num = 10;                // 用读取的url生成的测试数据的数目
uint32_t g_seed = 2222;                     // 随机种子
uint32_t g_block_size = 2 * 1024 * 1024;    // block的大小
uint32_t g_threshold_diff = 8;
uint32_t g_threshold_len = 12;
uint64_t g_max_cached_bytes = 20 * 1024;
uint32_t g_max_cached_block_nums = 80;
uint32_t g_reclaimed_bytes_threshold = 40;

CFLAGS_DEFINE_FLAG(uint32_t, url_num, cflags::BindTo(&g_url_num), "");
CFLAGS_DEFINE_FLAG(uint32_t, record_num, cflags::BindTo(&g_record_num), "");
CFLAGS_DEFINE_FLAG(uint32_t, threshold_diff, cflags::BindTo(&g_threshold_diff), "");
CFLAGS_DEFINE_FLAG(uint32_t, threshold_len, cflags::BindTo(&g_threshold_len), "");
CFLAGS_DEFINE_FLAG(uint64_t, max_cached_bytes, cflags::BindTo(&g_max_cached_bytes), "");
CFLAGS_DEFINE_FLAG(uint32_t, max_cached_block_nums,
                   cflags::BindTo(&g_max_cached_block_nums), "");
CFLAGS_DEFINE_FLAG(uint32_t, reclaimed_bytes_threshold,
                   cflags::BindTo(&g_reclaimed_bytes_threshold), "");

namespace sstable
{
class TestEnvironment : public testing::Environment
{
public:
    virtual void SetUp()
    {
        RAW_LOG(INFO, "TestEnvironment SetUp");
    }

    virtual void TearDown()
    {
        RAW_LOG(INFO, "TestEnvironment TearDown");
    }
};

class BlockCacheTest: public testing::Test
{
public:
    BlockCacheTest()
    {
        cache = NULL;
    }

    virtual ~BlockCacheTest()
    {
    }

protected:

    virtual void SetUp()
    {
        RAW_LOG(INFO, "WriterBlockTest SetUp");
    }

    virtual void TearDown()
    {
        RAW_LOG(INFO, "WriterBlockTest TearDown");
    }

    // @brief 生成4种测试用的数据，fkfv, fkvv, vkfv, vkvv;
    void InitTestData()
    {
        m_fixed_key_len = 20;
        m_fixed_value_len = 20;

        // fixed key, value
        m_test_helper[0].Init("url.dat", g_url_num, g_record_num, g_seed, m_fixed_key_len,
                                    m_fixed_value_len);

        // fixed key, var value
        m_test_helper[1].Init("url.dat", g_url_num, g_record_num, g_seed, m_fixed_key_len);

        // var key, fixed value
        m_test_helper[2].Init("url.dat", g_url_num, g_record_num, g_seed, 0, m_fixed_value_len);

        // var key, var value
        m_test_helper[3].Init("url.dat", g_url_num, g_record_num, g_seed);
    }

public:

    uint32_t m_fixed_key_len;       // fixed key length
    uint32_t m_fixed_value_len;     // fixed value length

    TestHelper m_test_helper[4];    // 用来测试四种block的测试数据
    BlockCache* cache;
}; // class WriteBlockTest

TEST_F(BlockCacheTest, InitAndDestroyCache)
{
    cache = BlockCache::GetCache();
    BlockCache::InitCache(g_max_cached_bytes,
                          g_max_cached_block_nums,
                          g_reclaimed_bytes_threshold);
    cache = BlockCache::GetCache();
    ASSERT_FALSE(NULL == cache);

    BlockCache::DestroyCache();
    cache = BlockCache::GetCache();
    ASSERT_TRUE(NULL == cache);
}

TEST_F(BlockCacheTest, GetHitRatio)
{
    BlockCache::InitCache(g_max_cached_bytes,
                          g_max_cached_block_nums,
                          g_reclaimed_bytes_threshold);
    cache = BlockCache::GetCache();
    ASSERT_TRUE(fabs(cache->GetHitRatio() - 0.0) < 0.00001);
}

TEST_F(BlockCacheTest, PushBlock)
{
    InitTestData();
    cache = BlockCache::GetCache();
    uint64_t file_id = 1;
    uint16_t block_no = 1;

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& records = m_test_helper[i].GetTestData();
        test_set_it iter;

        for (iter = records.begin(); iter != records.end();
            iter++, file_id++, block_no++)
        {
            char* block_buf = new char[(*iter).value->len];
            memcpy(block_buf, (*iter).value->data, (*iter).value->len);
            ASSERT_TRUE(cache->PushBlock(file_id, block_no,
                block_buf, (*iter).value->len));
        }
    }
}

TEST_F(BlockCacheTest, FetchBlock)
{
    cache = BlockCache::GetCache();
    char block_buffer[2 * 1024] = {0};
    uint32_t buffer_len = 2 * 1024;

    for (uint64_t i = 1; i <= 80; i += 10)
    {
        if (i <= 40)
        {
            ASSERT_TRUE(cache->FetchBlock(i , (uint16_t)i,
                        reinterpret_cast<char**>(&block_buffer), &buffer_len));
        }
        else
        {
            ASSERT_FALSE(cache->FetchBlock(i , (uint16_t)i,
                         reinterpret_cast<char**>(&block_buffer), &buffer_len));
        }
    }

    ASSERT_TRUE(fabs(cache->GetHitRatio() - 0.5) < 0.00001);
}

TEST_F(BlockCacheTest, Existed)
{
    cache = BlockCache::GetCache();
    for (int i = 1; i <= 80; i += 10)
    {
        if (i <= 40)
        {
            ASSERT_TRUE(cache->Existed(i , i));
        }
        else
        {
            ASSERT_FALSE(cache->Existed(i , i));
        }
    }
}

TEST_F(BlockCacheTest, ReleaseBlock)
{
    cache = BlockCache::GetCache();

    for (int i = 1; i <= 40; i += 10)
    {
        cache->ReleaseBlock(i, i);
        ASSERT_TRUE(cache->Existed(i , i));
        cache->ReleaseBlock(i, i);
        ASSERT_FALSE(cache->Existed(i , i));
    }
}

TEST_F(BlockCacheTest, ReclaimBlocks)
{
    cache = BlockCache::GetCache();
    for (int i = 2; i <= 40; i += 10)
    {
        ASSERT_TRUE(cache->Existed(i , i));
    }

    uint64_t cur_time = TimeUtils::Milliseconds();
    cache->ReclaimBlocks(cur_time);
    for (int i = 2; i <= 40; i += 10)
    {
        ASSERT_FALSE(cache->Existed(i , i));
    }
}

TEST_F(BlockCacheTest, ComputePriority)
{
    BlockCache::CachedUnit* cur_unit = new BlockCache::CachedUnit();
    assert(cur_unit != NULL);
    uint64_t cur_time = TimeUtils::Milliseconds();
    cur_unit->first_access_time = cur_time - 60 * 60 * 1000;
    cur_unit->num_hits = 3;

    cache = BlockCache::GetCache();
    cache->ComputePriority(cur_unit, cur_time);
    ASSERT_TRUE((cur_unit->priority - static_cast<double>(cur_unit->num_hits) /
              (60 * 60 * 1000 / BlockCache::kMilisecondsPerMin)) < 0.00001);

    delete cur_unit;
}

} // end of namespace sstable


int main(int argc, char** argv)
{
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);

    if (!cflags::ParseCommandLine(argc, argv))
    {
        return EXIT_FAILURE;
    }
    else
    {
        timeval time;
        gettimeofday(&time, NULL);
        g_seed = time.tv_sec + 1000 * time.tv_usec;
        // g_seed = 1345079175;
        printf("\nurl_num = %d, record_num = %d, seed = %d\n", g_url_num,
               g_record_num, g_seed);
        printf("threshold_len = %d, threshold_diff = %d\n\n", g_threshold_len,
               g_threshold_diff);
    }

    return RUN_ALL_TESTS();
}

