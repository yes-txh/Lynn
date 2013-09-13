/*
 * =====================================================================================
 *
 *       Filename:  multi_readers_iterator_test.cc
 *
 *    Description:  test for multi_readers_iterator.h
 *
 *        Version:  1.0
 *        Created:  01/14/2011 10:17:16 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  cyluo (xingLuo), 312733291@qq.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <sys/time.h>
#include <gtest/gtest.h>

#include "glog/logging.h"
#include "glog/raw_logging.h"
#include "common/config/cflags.hpp"
#include "common/base/string/string_algorithm.hpp"

#include "common/file/sstable/multi_sstables_reader.h"
#include "common/file/sstable/sstable_reader_iterator.h"
#include "common/file/sstable/multi_readers_iterator.h"
#include "common/file/sstable/sstable_writer.h"
#include "common/file/sstable/test_helper.h"

uint32_t g_url_num = 1000;                // 从文件中读取的url的数目
uint32_t g_record_num = 1000;           // 用读取的url生成的测试数据的数目
uint32_t g_seed = 2222;                 // 随机种子
uint32_t g_block_size = 2 * 1024;       // block的大小
uint32_t g_max_records = g_record_num;
uint32_t g_threshold_diff = 8;
uint32_t g_threshold_len = 12;
std::vector<std::string> g_files_name[4];

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

class MultiIteratorTest: public testing::Test
{
public:
    MultiIteratorTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            m_header[i] = NULL;
            m_multi_reader_table[i] = NULL;
        }
    }

    virtual ~MultiIteratorTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            delete m_header[i];
            delete m_multi_reader_table[i];
        }
    }

protected:

    virtual void SetUp()
    {
        RAW_LOG(INFO, "MultiIteratorTest SetUp");
    }

    virtual void TearDown()
    {
        RAW_LOG(INFO, "MultiIteratorTest TearDown");
    }

    // @brief 初始化一个定长key定长值的写入块
    // @param m_fixed_key_len
    // @param val_len
    // @param block_size
    // @param val_len
    // @param block_size
    // @retval
    void CreateFKFVHeader(uint16_t key_len,
                          uint32_t val_len,
                          uint32_t block_size,
                          sstable::CompressType compress_type,
                          sstable::SSTableHeader*& header_ptr)
    {
        header_ptr = new sstable::SSTableHeader();
        sstable::SSTableOptions* options_ptr = header_ptr->mutable_options();
        options_ptr->set_kv_type(sstable::kTypeFixedLen |
                                 (sstable::kTypeFixedLen << 4));
        options_ptr->set_compress_type(compress_type);
        options_ptr->set_fixed_key_len(key_len);
        options_ptr->set_fixed_data_len(val_len);
        options_ptr->set_block_size(block_size);
        options_ptr->set_threshold_diff(g_threshold_diff);
        options_ptr->set_threshold_len(g_threshold_len);
        options_ptr->set_bloomfilter_size(g_max_records);
    }

    // @brief 初始化一个定长Key变长值的写入块
    // @param m_fixed_key_len
    // @param block_size
    // @param compress_type
    // @param schema_ptr
    // @retval
    void CreateFKVVHeader(uint16_t key_len,
                          uint32_t block_size,
                          sstable::CompressType compress_type,
                          sstable::SSTableHeader*& header_ptr)
    {
        header_ptr = new sstable::SSTableHeader();
        sstable::SSTableOptions* options_ptr = header_ptr->mutable_options();
        options_ptr->set_kv_type(sstable::kTypeFixedLen |
                                 (sstable::kTypeVariableLen << 4));
        options_ptr->set_compress_type(compress_type);
        options_ptr->set_fixed_key_len(key_len);
        options_ptr->set_block_size(block_size);
        options_ptr->set_threshold_diff(g_threshold_diff);
        options_ptr->set_threshold_len(g_threshold_len);
        options_ptr->set_bloomfilter_size(g_max_records);
    }

    // @brief 初始化一个变长Key定长值的写入块
    // @param val_len
    // @param block_size
    // @param compress_type
    // @param schema_ptr
    // @retval
    void CreateVKFVHeader(uint32_t val_len,
                          uint32_t block_size,
                          sstable::CompressType compress_type,
                          sstable::SSTableHeader*& header_ptr)
    {
        header_ptr = new sstable::SSTableHeader();
        sstable::SSTableOptions* options_ptr = header_ptr->mutable_options();
        options_ptr->set_kv_type(sstable::kTypeVariableLen |
                                 (sstable::kTypeFixedLen << 4));
        options_ptr->set_compress_type(compress_type);
        options_ptr->set_fixed_data_len(val_len);
        options_ptr->set_block_size(block_size);
        options_ptr->set_threshold_diff(g_threshold_diff);
        options_ptr->set_threshold_len(g_threshold_len);
        options_ptr->set_bloomfilter_size(g_max_records);
    }

    // @brief 初始化一个变长Key定长值的写入块
    // @param block_size
    // @param compress_type
    // @param schema_ptr
    // @retval
    void CreateVKVVHeader(uint32_t block_size,
                          sstable::CompressType compress_type,
                          sstable::SSTableHeader*& header_ptr)
    {
        header_ptr = new sstable::SSTableHeader();
        sstable::SSTableOptions* options_ptr = header_ptr->mutable_options();
        options_ptr->set_kv_type(sstable::kTypeVariableLen |
                                (sstable::kTypeVariableLen << 4));
        options_ptr->set_compress_type(compress_type);
        options_ptr->set_block_size(block_size);
        options_ptr->set_threshold_diff(g_threshold_diff);
        options_ptr->set_threshold_len(g_threshold_len);
        options_ptr->set_bloomfilter_size(g_max_records);
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
    void InitTestData(int file_num)
    {
        m_fixed_key_len = 20;
        m_fixed_value_len = 20;

        for (int i = 0; i < 4; ++i)
        {
            g_files_name[i].clear();
        }

        for (int i = 0; i < file_num; ++i)
        {
            g_files_name[0].push_back(StringFormat("fkfv_table%d", i));
            g_files_name[1].push_back(StringFormat("fkvv_table%d", i));
            g_files_name[2].push_back(StringFormat("vkfv_table%d", i));
            g_files_name[3].push_back(StringFormat("vkvv_table%d", i));
        }

        // fixed key, value
        m_test_helper[0].Init("url.dat", g_url_num, g_record_num, g_seed,
            m_fixed_key_len, m_fixed_value_len);
        CreateFKFVHeader(m_fixed_key_len, m_fixed_value_len, g_block_size,
            intern::BlockCompressionCodec::LZO, m_header[0]);
        for (int i = 0; i < file_num; ++i)
        {
            SSTableWriter writer_table(g_files_name[0][i].c_str(),
                                        m_header[0]->options());

            test_set_t& records = m_test_helper[0].GetTestData();
            ASSERT_EQ(sstable::kRetOK, writer_table.Open());
            WritePBRecordsToTable(records, &writer_table);
            ASSERT_EQ(sstable::kRetOK, writer_table.Close());
        }


        // fixed key, var value
        m_test_helper[1].Init("url.dat", g_url_num, g_record_num, g_seed, m_fixed_key_len);
        CreateFKVVHeader(m_fixed_key_len, g_block_size,
            intern::BlockCompressionCodec::LZO, m_header[1]);
        for (int i = 0; i < file_num; ++i)
        {
            SSTableWriter writer_table(g_files_name[1][i].c_str(),
                                       m_header[1]->options());

            ASSERT_EQ(sstable::kRetOK, writer_table.Open());
            test_set_t& records = m_test_helper[1].GetTestData();
            WritePBRecordsToTable(records, &writer_table);
            ASSERT_EQ(sstable::kRetOK, writer_table.Close());
        }

        // var key, fixed value
        m_test_helper[2].Init("url.dat", g_url_num, g_record_num, g_seed, 0, m_fixed_value_len);
        CreateVKFVHeader(m_fixed_value_len, g_block_size,
            intern::BlockCompressionCodec::LZO, m_header[2]);
        for (int i = 0; i < file_num; ++i)
        {
            SSTableWriter  writer_table(g_files_name[2][i].c_str(),
                                        m_header[2]->options());

            ASSERT_EQ(sstable::kRetOK, writer_table.Open());
            test_set_t& records = m_test_helper[2].GetTestData();
            WritePBRecordsToTable(records, &writer_table);
            ASSERT_EQ(sstable::kRetOK, writer_table.Close());
        }

        // var key, var value
        m_test_helper[3].Init("url.dat", g_url_num, g_record_num, g_seed);
        CreateVKVVHeader(g_block_size,
            intern::BlockCompressionCodec::LZO, m_header[3]);
        for (int i = 0; i < file_num; ++i)
        {
            SSTableWriter  writer_table(g_files_name[3][i].c_str(),
                                        m_header[3]->options());

            ASSERT_EQ(sstable::kRetOK, writer_table.Open());
            test_set_t& records = m_test_helper[3].GetTestData();
            WritePBRecordsToTable(records, &writer_table);
            ASSERT_EQ(sstable::kRetOK, writer_table.Close());
        }

        for (int i = 0; i < 4; ++i)
        {
            m_multi_reader_table[i] = new MultiSSTablesReader(g_files_name[i]);
            ASSERT_EQ(sstable::kRetOK, m_multi_reader_table[i]->OpenFiles());
        }
    }

public:

    uint32_t m_fixed_key_len;           // fixed key length
    uint32_t m_fixed_value_len;         // fixed value length

    SSTableHeader*  m_header[4];        // 四种测试block对应的header
    MultiSSTablesReader* m_multi_reader_table[4];   // 四种测试reader
    TestHelper m_test_helper[4];        // 用来测试四种block的测试数据
}; // class MultiIteratorTest

// 测试NewIterator
TEST_F(MultiIteratorTest, New)
{
    InitTestData(10);

    for (int i = 0; i < 4; ++i)
    {
        MultiSSTablesReader::Iterator *iter = NULL;
        iter = m_multi_reader_table[i]->NewIterator();
        ASSERT_TRUE(NULL != iter);
        delete iter;

        ASSERT_EQ(kRetOK, m_multi_reader_table[i]->CloseFiles());
    }
}

// 测试CreateIterator
TEST_F(MultiIteratorTest, Create)
{
    InitTestData(10);

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

        MultiSSTablesReader::Iterator *iter = NULL;
        iter = m_multi_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        std::string key;
        iter->GetKey(&key);
        ASSERT_EQ(first_key, key);
        delete iter;

        ASSERT_EQ(kRetOK, m_multi_reader_table[i]->CloseFiles());
    }
}

// 测试Reset
TEST_F(MultiIteratorTest, Reset)
{
    InitTestData(10);

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

        MultiSSTablesReader::Iterator *iter = NULL;
        iter = m_multi_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (int j = 0; j < 100; j++)
        {
            iter->Next();
        }
        iter->Reset();

        std::string key;
        iter->GetKey(&key);
        ASSERT_EQ(first_key, key);
        delete iter;

        ASSERT_EQ(kRetOK, m_multi_reader_table[i]->CloseFiles());
    }
}

// 测试Next
TEST_F(MultiIteratorTest, Next)
{
    int file_num = 10;
    InitTestData(file_num);

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it data_iter;
        std::string test_key;
        std::string key;

        MultiSSTablesReader::Iterator *iter = NULL;
        iter = m_multi_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            test_key = std::string((*data_iter).key->data, (*data_iter).key->len);
            for (int j = 0; j < file_num; ++j)
            {
                iter->GetKey(&key);
                ASSERT_EQ(test_key, key);
                iter->Next();
            }
        }

        delete iter;

        ASSERT_EQ(kRetOK, m_multi_reader_table[i]->CloseFiles());
    }
}

// 测试Seek, LowBoundSeek
TEST_F(MultiIteratorTest, Seek_LowBoundSeek)
{
    int file_num = 4;
    InitTestData(file_num);

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

        MultiSSTablesReader::Iterator *iter = NULL;
        iter = m_multi_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            temp_key = std::string((*data_iter).key->data, (*data_iter).key->len);
            if (data_iter == data.begin() || first_key != temp_key)
            {
                first_key = temp_key;
                first_value = std::string((*data_iter).value->data, (*data_iter).value->len);
            }

            for (int j = 0; j < file_num; ++j)
            {
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
        }

        delete iter;

        ASSERT_EQ(kRetOK, m_multi_reader_table[i]->CloseFiles());
    }
}

// 测试Done
TEST_F(MultiIteratorTest, Done)
{
    int file_num = 10;
    InitTestData(file_num);

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        test_set_it data_iter;

        MultiSSTablesReader::Iterator *iter = NULL;
        iter = m_multi_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            for (int j = 0; j < file_num; ++j)
            {
                iter->Next();
            }
        }
        ASSERT_TRUE(iter->Done());
        delete iter;

        ASSERT_EQ(kRetOK, m_multi_reader_table[i]->CloseFiles());
    }
}

// 测试GetKey, GetValue, GetValuePtr
TEST_F(MultiIteratorTest, GetKey_GetValue)
{
    int file_num = 10;
    InitTestData(file_num);

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

        MultiSSTablesReader::Iterator *iter = NULL;
        iter = m_multi_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            test_key = std::string((*data_iter).key->data, (*data_iter).key->len);
            test_value = std::string((*data_iter).value->data, (*data_iter).value->len);

            for (int j = 0; j < file_num; ++j)
            {
                iter->GetKey(&key);
                iter->GetValue(&value);
                ASSERT_EQ(test_key, key);

                k_buffer_len = 10 * 1024;
                iter->GetKey(buffer, &k_buffer_len);
                key = std::string(buffer, k_buffer_len);
                ASSERT_EQ(test_key, key);

                buffer_len = 10 * 1024;
                iter->GetValue(buffer, &buffer_len);
                value = std::string(buffer, buffer_len);

                buffer_len = 10 * 1024;
                iter->GetValuePtr(NULL, &buffer_len);

                char* val_ptr = NULL;
                iter->GetValuePtr(&val_ptr, &buffer_len);
                value = std::string(val_ptr, buffer_len);

                StringPiece str_piece;
                StringPiece test_piece(test_value);
                iter->GetValuePtr(&str_piece);

                iter->Next();
            }
        }

        delete iter;
        ASSERT_EQ(kRetOK, m_multi_reader_table[i]->CloseFiles());
    }
}

// 测试GetKVPair
TEST_F(MultiIteratorTest, GetKVPair)
{
    int file_num = 10;
    InitTestData(file_num);
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

        MultiSSTablesReader::Iterator *iter = NULL;
        iter = m_multi_reader_table[i]->CreateIterator();
        ASSERT_TRUE(NULL != iter);

        for (data_iter = data.begin(); data_iter != data.end(); data_iter++)
        {
            test_key = std::string((*data_iter).key->data, (*data_iter).key->len);
            test_value = std::string((*data_iter).value->data, (*data_iter).value->len);

            for (int j = 0; j < file_num; ++j)
            {
                iter->GetKVPair(&key, &value);
                ASSERT_EQ(test_key, key);

                k_buffer_len = 10 * 1024;
                v_buffer_len = 10 * 1024;
                iter->GetKVPair(k_buffer, &k_buffer_len, v_buffer, &v_buffer_len);

                key = std::string(k_buffer, k_buffer_len);
                ASSERT_EQ(test_key, key);

                iter->Next();
            }
        }

        delete iter;

        ASSERT_EQ(kRetOK, m_multi_reader_table[i]->CloseFiles());
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

    return RUN_ALL_TESTS();
}


