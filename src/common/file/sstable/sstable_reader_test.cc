/*
 * =====================================================================================
 *
 *       Filename:  sstable_reader_test.cc
 *
 *    Description: test for sstable_reader
 *
 *        Version:  1.0
 *        Created:  01/07/2011 04:14:47 PM
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

#include "glog/logging.h"
#include "glog/raw_logging.h"
#include "common/file/sstable/test_helper.h"
#include "common/config/cflags.hpp"
#include "common/file/sstable/sstable_writer.h"
#include "common/file/sstable/sstable_reader.h"
#include "common/file/sstable/block_cache.h"
#include "common/file/sstable/sstable_reader_iterator.h"

uint32_t g_url_num = 10;                // 从文件中读取的url的数目
uint32_t g_record_num = 1000;           // 用读取的url生成的测试数据的数目
uint32_t g_seed = 2222;                 // 随机种子
uint32_t g_block_size = 2 * 1024;       // block的大小
uint32_t g_max_records = g_record_num;
uint32_t g_threshold_diff = 8;
uint32_t g_threshold_len = 12;
uint64_t g_max_cached_bytes = 10 * 1024;
uint32_t g_max_cached_block_nums = 80;
uint32_t g_reclaimed_bytes_threshold = 40;

CFLAGS_DEFINE_FLAG(uint32_t, url_num, cflags::BindTo(&g_url_num), "");
CFLAGS_DEFINE_FLAG(uint32_t, record_num, cflags::BindTo(&g_record_num), "");
CFLAGS_DEFINE_FLAG(uint32_t, threshold_diff, cflags::BindTo(&g_threshold_diff), "");
CFLAGS_DEFINE_FLAG(uint32_t, threshold_len, cflags::BindTo(&g_threshold_len), "");

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

class ReaderTest: public testing::Test
{
public:
    ReaderTest()
    {
        for (int i = 0; i < 5; ++i)
        {
            m_writer_table[i] = NULL;
            m_reader_table[i] = NULL;
        }
    }

    virtual ~ReaderTest()
    {
        for (int i = 0; i < 5; ++i)
        {
            delete m_writer_table[i];
            delete m_reader_table[i];
        }
    }

protected:

    virtual void SetUp()
    {
        RAW_LOG(INFO, "ReaderTest SetUp");
    }

    virtual void TearDown()
    {
        RAW_LOG(INFO, "ReaderTest TearDown");
    }

    // @brief 初始化一个定长key定长值的写入块
    // @param m_fixed_key_len
    // @param val_len
    // @param block_size
    // @param val_len
    // @retval
    SSTableWriter* CreateFKFVTable(uint16_t key_len,
                                   uint32_t val_len,
                                   uint32_t block_size,
                                   sstable::CompressType compress_type)
    {
        sstable::SSTableOptions options;
        options.set_kv_type(sstable::kTypeFixedLen |
                            (sstable::kTypeFixedLen << 4));
        options.set_compress_type(compress_type);
        options.set_fixed_key_len(key_len);
        options.set_fixed_data_len(val_len);
        options.set_block_size(block_size);
        options.set_threshold_diff(g_threshold_diff);
        options.set_threshold_len(g_threshold_len);
        options.set_bloomfilter_size(g_max_records);
        options.set_bloomfilter_prob(0.01);

        return new SSTableWriter("fkfv_table", options);
    }

    // @brief 初始化一个定长Key变长值的写入块
    // @param m_fixed_key_len
    // @param block_size
    // @param compress_type
    // @retval
    SSTableWriter* CreateFKVVTable(uint16_t key_len,
                                   uint32_t block_size,
                                   sstable::CompressType compress_type)
    {
        sstable::SSTableOptions options;
        options.set_kv_type(sstable::kTypeFixedLen |
                            (sstable::kTypeVariableLen << 4));
        options.set_compress_type(compress_type);
        options.set_fixed_key_len(key_len);
        options.set_block_size(block_size);
        options.set_threshold_diff(g_threshold_diff);
        options.set_threshold_len(g_threshold_len);
        options.set_bloomfilter_size(g_max_records);
        options.set_bloomfilter_prob(0.01);

        return new SSTableWriter("fkvv_table", options);
    }

    // @brief 初始化一个变长Key定长值的写入块
    // @param val_len
    // @param block_size
    // @param compress_type
    // @retval
    SSTableWriter* CreateVKFVTable(uint32_t val_len,
                                   uint32_t block_size,
                                   sstable::CompressType compress_type)
    {
        sstable::SSTableOptions options;
        options.set_kv_type(sstable::kTypeVariableLen |
                            (sstable::kTypeFixedLen << 4));
        options.set_compress_type(compress_type);
        options.set_fixed_data_len(val_len);
        options.set_block_size(block_size);
        options.set_threshold_diff(g_threshold_diff);
        options.set_threshold_len(g_threshold_len);
        options.set_bloomfilter_size(g_max_records);
        options.set_bloomfilter_prob(0.01);

        return new SSTableWriter("vkfv_table", options);
    }

    // @brief 初始化一个变长Key定长值的写入块
    // @param block_size
    // @param compress_type
    // @retval
    SSTableWriter* CreateVKVVTable(uint32_t block_size,
                                   sstable::CompressType compress_type)
    {
        sstable::SSTableOptions options;
        options.set_kv_type(sstable::kTypeVariableLen |
                            (sstable::kTypeVariableLen << 4));
        options.set_compress_type(compress_type);
        options.set_block_size(block_size);
        options.set_threshold_diff(g_threshold_diff);
        options.set_threshold_len(g_threshold_len);
        options.set_bloomfilter_size(g_max_records);
        options.set_bloomfilter_prob(0.01);

        return new SSTableWriter("vkvv_table", options);
    }

    // @brief 初始化一个默认配置的写入块
    // @retval
    SSTableWriter* CreateDefaultTable()
    {
        sstable::SSTableOptions options;
        return new SSTableWriter("default_table", options);
    }

    // @brief 写入一组数据到目标block当中;
    // @param records,
    // @param target_block,
    // @retval
    void WritePBRecordsToTable(test_set_t& records, SSTableWriter* target_table)
    {
        test_set_it iter;
        for (iter = records.begin(); iter != records.end(); iter++)
        {
            ASSERT_EQ(sstable::kRetOK,
                      target_table->WriteRecord((*iter).key->data,
                                                (*iter).key->len,
                                                (*iter).value->data,
                                                (*iter).value->len));
        }
    }


    // @brief 生成5种测试用的数据，fkfv, fkvv, vkfv, vkvv, default;
    void InitTestData()
    {
        m_fixed_key_len = 20;
        m_fixed_value_len = 20;

        // fixed key, value
        m_writer_table[0]= CreateFKFVTable(m_fixed_key_len, m_fixed_value_len, g_block_size,
                                    intern::BlockCompressionCodec::LZO);
        m_test_helper[0].Init("url.dat", g_url_num, g_record_num, g_seed,
                                    m_fixed_key_len, m_fixed_value_len);
        m_reader_table[0] = new SSTableReader("fkfv_table");

        // fixed key, var value
        m_writer_table[1]= CreateFKVVTable(m_fixed_key_len, g_block_size,
                                    intern::BlockCompressionCodec::LZO);
        m_test_helper[1].Init("url.dat", g_url_num, g_record_num, g_seed, m_fixed_key_len);
        m_reader_table[1] = new SSTableReader("fkvv_table");

        // var key, fixed value
        m_writer_table[2]= CreateVKFVTable(m_fixed_value_len, g_block_size,
                                    intern::BlockCompressionCodec::LZO);
        m_test_helper[2].Init("url.dat", g_url_num, g_record_num, g_seed, 0, m_fixed_value_len);
        m_reader_table[2] = new SSTableReader("vkfv_table");

        // var key, var value
        m_writer_table[3]= CreateVKVVTable(g_block_size,
                                    intern::BlockCompressionCodec::LZO);
        m_test_helper[3].Init("url.dat", g_url_num, g_record_num, g_seed);
        m_reader_table[3] = new SSTableReader("vkvv_table");

        // default
        m_writer_table[4]= CreateDefaultTable();
        m_test_helper[4].Init("url.dat", g_url_num, g_record_num, g_seed);
        m_reader_table[4] = new SSTableReader("default_table");

        for (int i = 0; i < 5; ++i)
        {
            ASSERT_EQ(sstable::kRetOK, m_writer_table[i]->Open());
            test_set_t& records = m_test_helper[i].GetTestData();
            WritePBRecordsToTable(records, m_writer_table[i]);
            ASSERT_EQ(sstable::kRetOK, m_writer_table[i]->Close());
        }
    }

public:

    uint32_t m_fixed_key_len;           // fixed key length
    uint32_t m_fixed_value_len;         // fixed value length

    SSTableWriter* m_writer_table[5];   // 5种测试writer_block
    SSTableReader* m_reader_table[5];   // 5种测试reader_block
    TestHelper m_test_helper[5];        // 用来测试5种block的测试数据
}; // class WriteTest

TEST_F(ReaderTest, Open_Writer_Close)
{
    InitTestData();

    for (int i = 0; i < 5; ++i)
    {
        ASSERT_EQ(sstable::kRetOK, m_reader_table[i]->OpenFile()) << "file index: " << i;
    }

    for (int i = 0; i < 5; ++i)
    {
        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

TEST_F(ReaderTest, MemMap)
{
    InitTestData();

    int i = 0;
    for (i = 0; i < 5; ++i)
    {
        ASSERT_EQ(sstable::kRetOK, m_reader_table[i]->OpenFile());
        ASSERT_EQ(sstable::kRetOK, m_reader_table[i]->MemMap());
    }


    for (i = 0; i < 5; ++i)
    {
        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}


TEST_F(ReaderTest, LoadDataBlock)
{
    InitTestData();

    int i = 0;
    for (i = 0; i < 5; ++i)
    {
        ASSERT_EQ(sstable::kRetOK, m_reader_table[i]->OpenFile());

        uint32_t num_blocks = m_reader_table[i]->GetNumBlocks();
        for (uint32_t j = 0; j < num_blocks; ++j)
        {
            ASSERT_TRUE(NULL != m_reader_table[i]->LoadDataBlock(j));
            m_reader_table[i]->ClearBlock(j);
        }
    }

    for (i = 0; i < 5; ++i)
    {
        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

TEST_F(ReaderTest, LoadBlockWithPrefetch)
{
    InitTestData();

    int i = 0;

    // without cache
    for (i = 0; i < 5; ++i)
    {
        ASSERT_EQ(sstable::kRetOK, m_reader_table[i]->OpenFile());

        uint32_t num_blocks = m_reader_table[i]->GetNumBlocks();
        for (uint32_t j = 0; j < num_blocks; ++j)
        {
            ASSERT_TRUE(NULL != m_reader_table[i]->LoadBlockWithPrefetch(j));
            m_reader_table[i]->ClearBlock(j);
        }
    }

    // with cache
    BlockCache::InitCache(g_max_cached_bytes,
                          g_max_cached_block_nums,
                          g_reclaimed_bytes_threshold);
    for (i = 0; i < 5; ++i)
    {
        uint32_t num_blocks = m_reader_table[i]->GetNumBlocks();

        // load from file
        for (uint32_t j = 0; j < num_blocks; ++j)
        {
            ASSERT_TRUE(NULL != m_reader_table[i]->LoadBlockWithPrefetch(j));
            m_reader_table[i]->ClearBlock(j);
        }
    }

    BlockCache::DestroyCache();

    for (i = 0; i < 5; ++i)
    {
        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

TEST_F(ReaderTest, FindFirstValue)
{
    InitTestData();

    for (int32_t i = 0; i < 5; ++i)
    {
        ASSERT_EQ(sstable::kRetOK, m_reader_table[i]->OpenFile());
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it iter;

        std::string str_first_same_key;
        std::string str_first_same_val;
        char first_same_val[1024 * 10] = {0};
        uint32_t first_same_val_len = 1024 * 10;
        std::string str_cur_val;
        char cur_val[1024 * 10] = {0};
        uint32_t cur_val_len = 1024 * 10;

        for (iter = data.begin(); iter != data.end(); iter++)
        {
            std::string test_key((*iter).key->data, (*iter).key->len);
            ASSERT_EQ(kRetOK, m_reader_table[i]->FindFirstValue((*iter).key->data,
                      (*iter).key->len, cur_val, &cur_val_len));
            ASSERT_TRUE(m_reader_table[i]->FindFirstValue((*iter).key->data,
                (*iter).key->len, &str_cur_val));
            if (iter == data.begin() || test_key != str_first_same_key)
            {
                str_first_same_key = test_key;
                str_first_same_val = str_cur_val;
                memcpy(first_same_val, cur_val, cur_val_len);
                first_same_val_len = cur_val_len;
            }
            else
            {
                ASSERT_EQ(str_first_same_val, str_cur_val);
                ASSERT_EQ(cur_val_len, first_same_val_len);
                ASSERT_EQ(0, strncmp(cur_val, first_same_val, first_same_val_len));
            }

            cur_val_len = 1024 * 10;

            char not_find_key[1024 * 2] = {0};
            memcpy(not_find_key, (*iter).key->data, (*iter).key->len);
            not_find_key[(*iter).key->len - 1] += 1;

            ASSERT_EQ(kRetRecordNotFind,
                m_reader_table[i]->FindFirstValue(not_find_key,
                (*iter).key->len, cur_val, &cur_val_len));
            ASSERT_FALSE(m_reader_table[i]->FindFirstValue(not_find_key,
                (*iter).key->len, &str_cur_val));

            cur_val_len = first_same_val_len - 1;
            ASSERT_EQ(kRetBufferOverflow,
                m_reader_table[i]->FindFirstValue((*iter).key->data,
                (*iter).key->len, cur_val, &cur_val_len));

            cur_val_len = 1024 * 10;
        }
    }

    for (int32_t i = 0; i < 5; ++i)
    {
        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

TEST_F(ReaderTest, SeekRecord)
{
    InitTestData();

    for (int32_t i = 0; i < 5; ++i)
    {
        ASSERT_EQ(sstable::kRetOK, m_reader_table[i]->OpenFile());
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it iter;

        ReaderBlock* block = NULL;
        uint32_t block_no = 0;
        uint32_t record_no = 0;
        uint32_t basekey_record_no = 0;

        for (iter = data.begin(); iter != data.end(); iter++)
        {
            ASSERT_TRUE(m_reader_table[i]->SeekRecord((*iter).key->data,
                        (*iter).key->len, &block, &block_no,
                        &record_no, &basekey_record_no));
            m_reader_table[i]->ClearBlock(block_no);

            char not_find_key[1024 * 2] = {0};
            memcpy(not_find_key, (*iter).key->data, (*iter).key->len);
            not_find_key[(*iter).key->len - 1] += 1;

            ASSERT_FALSE(m_reader_table[i]->SeekRecord(not_find_key,
                         (*iter).key->len, &block, &block_no,
                         &record_no, &basekey_record_no));
        }
    }

    for (int32_t i = 0; i < 5; ++i)
    {
        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

TEST_F(ReaderTest, LowBoundSeekRecord)
{
    InitTestData();

    for (int32_t i = 0; i < 5; ++i)
    {
        ASSERT_EQ(sstable::kRetOK, m_reader_table[i]->OpenFile());
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it iter;

        ReaderBlock* block = NULL;
        uint32_t block_no = 0;
        uint32_t record_no = 0;
        uint32_t basekey_record_no = 0;
        bool is_accurate_hit = false;

        for (iter = data.begin(); iter != data.end(); iter++)
        {
            ASSERT_TRUE(m_reader_table[i]->LowBoundSeekRecord((*iter).key->data,
                        (*iter).key->len, &block, &block_no,
                        &record_no, &basekey_record_no, &is_accurate_hit));
            ASSERT_TRUE(is_accurate_hit);
            m_reader_table[i]->ClearBlock(block_no);

            char not_accurate_hit_key[1024 * 2] = {0};
            memcpy(not_accurate_hit_key, (*iter).key->data, (*iter).key->len);
            not_accurate_hit_key[(*iter).key->len - 1] -= 1;

            ASSERT_TRUE(m_reader_table[i]->LowBoundSeekRecord(not_accurate_hit_key,
                        (*iter).key->len, &block, &block_no,
                        &record_no, &basekey_record_no, &is_accurate_hit));
            ASSERT_FALSE(is_accurate_hit);
            m_reader_table[i]->ClearBlock(block_no);

            if ( i < 2 )
            {
                char not_find_key[1024 * 2] = {0};
                memcpy(not_find_key, (*iter).key->data, (*iter).key->len);
                not_find_key[0] += 1;
                ASSERT_FALSE(m_reader_table[i]->LowBoundSeekRecord(not_find_key,
                         (*iter).key->len, &block, &block_no,
                         &record_no, &basekey_record_no, &is_accurate_hit));
            }
        }
    }

    for (int32_t i = 0; i < 5; ++i)
    {
        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
};

TEST_F(ReaderTest, NewIterator)
{
    InitTestData();

    for (int i = 0; i < 5; ++i)
    {
        ASSERT_EQ(sstable::kRetOK, m_reader_table[i]->OpenFile());
        SSTableReader::Iterator *iter = m_reader_table[i]->NewIterator();
        ASSERT_TRUE(NULL != iter);
        delete iter;
        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

TEST_F(ReaderTest, CreateIterator)
{
    InitTestData();

    for (int i = 0; i < 5; ++i)
    {
        ASSERT_EQ(sstable::kRetOK, m_reader_table[i]->OpenFile());
        SSTableReader::Iterator *iter = m_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);
        delete iter;
        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

} // namespace sstable


int main(int argc, char** argv)
{
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    testing::GTEST_FLAG(break_on_failure) = 1;

    FLAGS_log_dir = "./";
    FLAGS_stderrthreshold = 3;
    FLAGS_logbuflevel = -1;

    if (!cflags::ParseCommandLine(argc, argv))
    {
        return EXIT_FAILURE;
    }
    else
    {
        timeval time;
        gettimeofday(&time, NULL);
        g_seed = time.tv_sec + 1000 * time.tv_usec;
        g_seed = -2086793282;
        printf("\nurl_num = %d, record_num = %d, seed = %d\n", g_url_num,
               g_record_num, g_seed);
        printf("threshold_len = %d, threshold_diff = %d\n\n", g_threshold_len,
               g_threshold_diff);
    }

    return RUN_ALL_TESTS();
}
