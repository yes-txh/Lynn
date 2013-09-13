/*
 * =====================================================================================
 *
 *       Filename:  sstable_reader_iterator_test.cc
 *
 *    Description:  test for sstable_reader_iterator.h
 *
 *        Version:  1.0
 *        Created:  01/11/2011 06:30:55 PM
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

#include <string>

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
uint64_t g_max_cached_bytes = 2 << 20;
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

class ReaderIteratorTest: public testing::Test
{
public:
    ReaderIteratorTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            m_writer_table[i] = NULL;
            m_reader_table[i] = NULL;
        }
    }

    virtual ~ReaderIteratorTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            delete m_writer_table[i];
            delete m_reader_table[i];
        }
    }

protected:

    virtual void SetUp()
    {
        RAW_LOG(INFO, "ReaderIteratorTest SetUp");
    }

    virtual void TearDown()
    {
        RAW_LOG(INFO, "ReaderIteratorTest TearDown");
    }

    // @brief 初始化一个定长key定长值的写入块
    // @param m_fixed_key_len
    // @param val_len
    // @param block_size
    // @param val_len
    // @param block_size
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
    // @param schema_ptr
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
    // @param schema_ptr
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
    // @param schema_ptr
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


    // @brief 生成4种测试用的数据，fkfv, fkvv, vkfv, vkvv;
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

        for (int i = 0; i < 4; ++i)
        {
            ASSERT_EQ(sstable::kRetOK, m_writer_table[i]->Open());
            test_set_t& records = m_test_helper[i].GetTestData();
            WritePBRecordsToTable(records, m_writer_table[i]);
            ASSERT_EQ(sstable::kRetOK, m_writer_table[i]->Close());

            ASSERT_EQ(sstable::kRetOK, m_reader_table[i]->OpenFile());
        }
    }

public:

    uint32_t m_fixed_key_len;           // fixed key length
    uint32_t m_fixed_value_len;         // fixed value length

    SSTableWriter* m_writer_table[4];   // 四种测试writer_block
    SSTableReader* m_reader_table[4];   // 四种测试reader_block
    TestHelper m_test_helper[4];        // 用来测试四种block的测试数据
}; // class WriteTest

TEST_F(ReaderIteratorTest, New)
{
    InitTestData();

    for (int i = 0; i < 4; ++i)
    {
        SSTableReaderIterator *iter = NULL;
        iter = m_reader_table[i]->NewIterator();
        ASSERT_TRUE(NULL != iter);
        delete iter;

        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

TEST_F(ReaderIteratorTest, Create)
{
    InitTestData();

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it data_iter;
        std::string first_key;

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            first_key = std::string((*data_iter).key->data, (*data_iter).key->len);
            break;
        }

        SSTableReader::Iterator *iter = NULL;
        iter = m_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        std::string key;
        iter->GetKey(&key);
        ASSERT_EQ(first_key, key);
        delete iter;

        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}


TEST_F(ReaderIteratorTest, Reset)
{
    InitTestData();

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it data_iter;
        std::string first_key;

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            first_key = std::string((*data_iter).key->data, (*data_iter).key->len);
            break;
        }

        SSTableReader::Iterator *iter = NULL;
        iter = m_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (int j = 0; j < 10; j++)
        {
            iter->Next();
        }
        iter->Reset();

        std::string key;
        iter->GetKey(&key);
        ASSERT_EQ(first_key, key);
        delete iter;

        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}


TEST_F(ReaderIteratorTest, Next)
{
    InitTestData();

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it data_iter;
        std::string test_key;
        std::string key;

        SSTableReader::Iterator *iter = NULL;
        iter = m_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            test_key = std::string((*data_iter).key->data, (*data_iter).key->len);
            iter->GetKey(&key);
            ASSERT_EQ(test_key, key);
            iter->Next();
        }

        delete iter;

        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

TEST_F(ReaderIteratorTest, Seek_LowBoundSeek)
{
    InitTestData();
    char buffer[10 * 1024];
    uint32_t buffer_len = 10 * 1024;

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it data_iter;
        std::string first_key;
        std::string first_value;
        std::string key;
        std::string value;
        std::string temp_key;

        SSTableReader::Iterator *iter = NULL;
        iter = m_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            temp_key = std::string((*data_iter).key->data, (*data_iter).key->len);
            if (data_iter == data.begin() || first_key != temp_key)
            {
                first_key = temp_key;
                first_value = std::string((*data_iter).value->data, (*data_iter).value->len);
            }

            iter->Seek((*data_iter).key->data, (*data_iter).key->len);
            iter->GetKey(&key);
            iter->GetValue(&value);
            ASSERT_EQ(first_key, key);
            ASSERT_EQ(first_value, value);

            buffer_len = (*data_iter).key->len;
            memcpy(buffer, (*data_iter).key->data, buffer_len);
            buffer[buffer_len - 1] -= 1;
            iter->LowBoundSeek(buffer, buffer_len);
            iter->GetKey(&key);
            iter->GetValue(&value);
            ASSERT_EQ(first_key, key);
            ASSERT_EQ(first_value, value);

            iter->Next();
        }

        delete iter;

        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}


TEST_F(ReaderIteratorTest, Done)
{
    InitTestData();

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it data_iter;

        SSTableReader::Iterator *iter = NULL;
        iter = m_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            iter->Next();
        }
        ASSERT_TRUE(iter->Done());

        delete iter;

        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

TEST_F(ReaderIteratorTest, GetKey_GetValue)
{
    InitTestData();
    char buffer[10 * 1024];
    uint32_t buffer_len = 10 * 1024;
    uint16_t k_buffer_len = 10 * 1024;

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it data_iter;
        std::string test_key;
        std::string test_value;
        std::string key;
        std::string value;

        SSTableReader::Iterator *iter = NULL;
        iter = m_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            test_key = std::string((*data_iter).key->data, (*data_iter).key->len);
            test_value = std::string((*data_iter).value->data, (*data_iter).value->len);

            iter->GetKey(&key);
            iter->GetValue(&value);
            ASSERT_EQ(test_key, key);
            ASSERT_EQ(test_value, value);

            k_buffer_len = 10 * 1024;
            iter->GetKey(buffer, &k_buffer_len);
            key = std::string(buffer, k_buffer_len);
            ASSERT_EQ(test_key, key);
            buffer_len = 10 * 1024;
            iter->GetValue(buffer, &buffer_len);
            value = std::string(buffer, buffer_len);
            ASSERT_EQ(test_value, value);

            buffer_len = 10 * 1024;
            iter->GetValuePtr(NULL, &buffer_len);
            ASSERT_EQ((*data_iter).value->len, buffer_len);

            char* val_ptr = NULL;
            iter->GetValuePtr(&val_ptr, &buffer_len);
            value = std::string(val_ptr, buffer_len);
            ASSERT_EQ(test_value, value);

            StringPiece str_piece;
            StringPiece test_piece(test_value);
            iter->GetValuePtr(&str_piece);
            ASSERT_TRUE(str_piece == test_piece);

            iter->Next();
        }

        delete iter;

        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}


TEST_F(ReaderIteratorTest, GetKVPair)
{
    InitTestData();
    char k_buffer[10 * 1024];
    char v_buffer[10 * 1024];
    uint32_t v_buffer_len = 10 * 1024;
    uint16_t k_buffer_len = 10 * 1024;

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it data_iter;
        std::string test_key;
        std::string test_value;
        std::string key;
        std::string value;

        SSTableReader::Iterator *iter = NULL;
        iter = m_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            test_key = std::string((*data_iter).key->data, (*data_iter).key->len);
            test_value = std::string((*data_iter).value->data, (*data_iter).value->len);

            iter->GetKVPair(&key, &value);
            ASSERT_EQ(test_key, key);
            ASSERT_EQ(test_value, value);

            k_buffer_len = 10 * 1024;
            v_buffer_len = 10 * 1024;
            iter->GetKVPair(k_buffer, &k_buffer_len, v_buffer, &v_buffer_len);

            key = std::string(k_buffer, k_buffer_len);
            ASSERT_EQ(test_key, key);

            value = std::string(v_buffer, v_buffer_len);
            ASSERT_EQ(test_value, value);

            iter->Next();
        }

        delete iter;

        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

TEST_F(ReaderIteratorTest, GreaterThan)
{
    InitTestData();
    std::string key_m;
    std::string key_n;

    for (int i = 0; i < 4; ++i)
    {
        SSTableReader::Iterator *iter_m = NULL;
        iter_m = m_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter_m);

        while (!iter_m->Done())
        {
            iter_m->GetKey(&key_m);
            SSTableReader::Iterator *iter_n = NULL;
            iter_n = m_reader_table[i]->CreateIterator();
            ASSERT_TRUE(NULL != iter_n);

            while (!iter_n->Done())
            {
                iter_n->GetKey(&key_n);
                if (key_m <= key_n)
                {
                    ASSERT_FALSE(iter_m->GreaterThan(iter_n));
                }
                else
                {
                    ASSERT_TRUE(iter_m->GreaterThan(iter_n));
                }

                iter_n->Next();
            }

            delete iter_n;

            iter_m->Next();
        }

        delete iter_m;
        ASSERT_EQ(kRetOK, m_reader_table[i]->CloseFile());
    }
}

} // namespace sstable


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
        // g_seed = -2086793282;
        printf("\nurl_num = %d, record_num = %d, seed = %d\n", g_url_num,
               g_record_num, g_seed);
        printf("threshold_len = %d, threshold_diff = %d\n\n", g_threshold_len,
               g_threshold_diff);
    }

    // with cache
    sstable::BlockCache::InitCache(g_max_cached_bytes,
                          g_max_cached_block_nums,
                          g_reclaimed_bytes_threshold);
    int ret = RUN_ALL_TESTS();

    sstable::BlockCache::DestroyCache();
    return ret;
}
