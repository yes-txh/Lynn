/*
 * =====================================================================================
 *
 *       Filename:  sstable_reader_block_test.cc
 *
 *    Description:  test for sstable_reader_block.cc
 *
 *        Version:  1.0
 *        Created:  01/05/2011 08:20:22 PM
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
#include "common/file/sstable/sstable_reader_block.h"
#include "common/file/sstable/sstable_writer_block.h"

uint32_t g_url_num = 10;                    // 从文件中读取的url的数目
uint32_t g_record_num = 1000;               // 用读取的url生成的测试数据的数目
uint32_t g_seed = 2222;                     // 随机种子
uint32_t g_block_size = 2 * 1024 * 1024;    // block的大小
uint32_t g_threshold_diff = 8;
uint32_t g_threshold_len = 12;

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

class ReaderBlockTest: public testing::Test
{
public:
    ReaderBlockTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            m_header[i] = NULL;
            m_writer_block[i] = NULL;
            m_reader_block[i] = NULL;
        }
    }

    virtual ~ReaderBlockTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            delete m_header[i];
            delete m_writer_block[i];
            delete m_reader_block[i];
            m_test_helper[i].UnInit();
        }
    }

protected:

    virtual void SetUp()
    {
        RAW_LOG(INFO, "ReaderBlockTest SetUp");
    }

    virtual void TearDown()
    {
        RAW_LOG(INFO, "ReaderBlockTest TearDown");
    }

    // @brief 初始化一个定长key定长值的写入块
    // @param m_fixed_key_len
    // @param val_len
    // @param block_size
    // @param val_len
    // @param block_size
    // @retval
    WriterBlock* CreateFKFVBlock(uint16_t key_len,
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

        return new WriterBlock(
                   const_cast< const sstable::SSTableHeader* >(header_ptr));
    }

    // @brief 初始化一个定长Key变长值的写入块
    // @param m_fixed_key_len
    // @param block_size
    // @param compress_type
    // @param schema_ptr
    // @retval
    WriterBlock* CreateFKVVBlock(uint16_t key_len,
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

        return new WriterBlock(
                   const_cast< const sstable::SSTableHeader* >(header_ptr));
    }

    // @brief 初始化一个变长Key定长值的写入块
    // @param val_len
    // @param block_size
    // @param compress_type
    // @param schema_ptr
    // @retval
    WriterBlock* CreateVKFVBlock(uint32_t val_len,
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

        return new WriterBlock(
                   const_cast< const sstable::SSTableHeader* >(header_ptr));
    }

    // @brief 初始化一个变长Key定长值的写入块
    // @param block_size
    // @param compress_type
    // @param schema_ptr
    // @retval
    WriterBlock* CreateVKVVBlock(uint32_t block_size,
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

        return new WriterBlock(
                   const_cast< const sstable::SSTableHeader* >(header_ptr));
    }

    // @brief 写入一组数据到目标block当中;
    // @param records,
    // @param target_block,
    // @retval
    void WritePBRecordsToBlock(test_set_t& records, WriterBlock* target_block)
    {
        test_set_it iter;
        for (iter = records.begin(); iter != records.end(); iter++)
        {
            ASSERT_EQ(sstable::kRetOK,
                      target_block->WriteRecord((*iter).key->data,
                                                (*iter).key->len,
                                                (*iter).value->data,
                                                (*iter).value->len));
        }
    }

    // @brief 写入一组数据到目标block当中;
    void InitReaderBlock()
    {
        InitTestData();

        char buffer[2 * 1024 * 1024];
        uint32_t buffer_len = 2 * 1024 * 1024;

        for (int i = 0; i < 4; ++i)
        {
            // 压缩WriterBlock中的数据
            buffer_len = g_block_size;
            ASSERT_EQ(sstable::kRetOK,
                      m_writer_block[i]->CompressBlockData(buffer, &buffer_len));

            // 解压缩数据到ReaderBlock中
            ASSERT_EQ(sstable::kRetOK,
                      m_reader_block[i]->UnCompressBlockData(buffer, buffer_len));
        }
    }

    // @brief 生成4种测试用的数据，fkfv, fkvv, vkfv, vkvv;
    void InitTestData()
    {
        m_fixed_key_len = 20;
        m_fixed_value_len = 20;

        // fixed key, value
        m_writer_block[0]= CreateFKFVBlock(m_fixed_key_len, m_fixed_value_len,
                              g_block_size, intern::BlockCompressionCodec::LZO, m_header[0]);
        m_test_helper[0].Init("url.dat", g_url_num, g_record_num, g_seed,
                              m_fixed_key_len, m_fixed_value_len);

        // fixed key, var value
        m_writer_block[1]= CreateFKVVBlock(m_fixed_key_len, g_block_size,
                              intern::BlockCompressionCodec::LZO, m_header[1]);
        m_test_helper[1].Init("url.dat", g_url_num, g_record_num, g_seed,
                              m_fixed_key_len);

        // var key, fixed value
        m_writer_block[2]= CreateVKFVBlock(m_fixed_value_len, g_block_size,
                              intern::BlockCompressionCodec::LZO, m_header[2]);
        m_test_helper[2].Init("url.dat", g_url_num, g_record_num, g_seed, 0,
                              m_fixed_value_len);

        // var key, var value
        m_writer_block[3]= CreateVKVVBlock(g_block_size,
                              intern::BlockCompressionCodec::LZO, m_header[3]);
        m_test_helper[3].Init("url.dat", g_url_num, g_record_num, g_seed);

        for (int i = 0; i < 4; ++i)
        {
            test_set_t& data = m_test_helper[i].GetTestData();
            WritePBRecordsToBlock(data, m_writer_block[i]);
            m_reader_block[i] = new ReaderBlock(m_header[i]);
        }
    }

    void CheckReaderBlock(ReaderBlock* reader_block, test_set_t& records,
                          int test_type)
    {
        test_set_it iter;
        uint32_t cur_record_no = 0;
        uint32_t cur_base_index = 0;
        uint32_t next_base_index = 0;
        for (iter = records.begin(); iter != records.end(); iter++)
        {
            // get next base index
            if (test_type >= 2)
            {
                next_base_index = reader_block->GetNextBaseIndex(
                                      cur_record_no, cur_base_index);
            }

            // check key
            std::string cur_string;
            std::string test_key((*iter).key->data, (*iter).key->len);
            reader_block->GetKeyByRecordNo(cur_record_no,
                                           cur_base_index, &cur_string);
            ASSERT_EQ(cur_string, test_key);

            // check value
            std::string test_value((*iter).value->data, (*iter).value->len);
            reader_block->GetValByRecordNo(cur_record_no, &cur_string);
            ASSERT_EQ(cur_string, test_value);

            cur_base_index = next_base_index;
            cur_record_no++;
        }
    }

public:

    uint32_t m_fixed_key_len;       // fixed key length
    uint32_t m_fixed_value_len;     // fixed value length

    SSTableHeader*  m_header[4];    // 四种测试block对应的block_header
    WriterBlock* m_writer_block[4]; // 四种测试block
    ReaderBlock* m_reader_block[4]; // 四种测试block
    TestHelper m_test_helper[4];    // 用来测试四种block的测试数据
}; // class WriteBlockTest

// 对UnCompressBlockData进行测试
TEST_F(ReaderBlockTest, UnCompressBlockData)
{
    InitTestData();
    char buffer[2 * 1024 * 1024];
    uint32_t buffer_len = 2 * 1024 *1024;

    for (int i = 0; i < 4; ++i)
    {
         // 压缩WriterBlock中的数据
        buffer_len = g_block_size;
        ASSERT_EQ(sstable::kRetOK,
                  m_writer_block[i]->CompressBlockData(buffer, &buffer_len));

         // 解压缩数据到ReaderBlock中
        ASSERT_EQ(sstable::kRetOK,
                  m_reader_block[i]->UnCompressBlockData(buffer, buffer_len));

         // 验证ReaderBlock中的数据
        test_set_t& data = m_test_helper[i].GetTestData();
        CheckReaderBlock(m_reader_block[i], data, i);
    }
}

// 测试GetPrefixCompressedKey
TEST_F(ReaderBlockTest, GetPrefixCompressedKey)
{
    // 初始化m_reader_block
    InitReaderBlock();

    PrefixCompressedKey pre_fix_comp_key;
    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();

        test_set_it iter;
        uint32_t cur_record_no = 0;
        uint32_t cur_base_index = 0;
        uint32_t next_base_index = 0;
        for (iter = data.begin(); iter != data.end(); iter++)
        {
            // get next base index
            if (i >= 2)
            {
                next_base_index = m_reader_block[i]->GetNextBaseIndex(
                                      cur_record_no, cur_base_index);
            }

            m_reader_block[i]->GetPrefixCompressedKey(cur_record_no, cur_base_index,
                    &pre_fix_comp_key);

            // check key
            std::string cur_string;
            std::string test_key((*iter).key->data, (*iter).key->len);
            cur_string = std::string(pre_fix_comp_key.prefix_key,
                                     pre_fix_comp_key.prefix_key_len) +
                         std::string(pre_fix_comp_key.remainder_key,
                                     pre_fix_comp_key.remainder_key_len);
            ASSERT_EQ(cur_string, test_key);

            cur_base_index = next_base_index;
            cur_record_no++;
        }
    }
}


// 测试GetKeyByRecordNo
TEST_F(ReaderBlockTest, GetKeyByRecordNo)
{
    // 初始化m_reader_block
    InitReaderBlock();
    char buffer[1024 * 10];
    uint16_t buffer_len = 1024 * 10;

    PrefixCompressedKey pre_fix_comp_key;
    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();

        test_set_it iter;
        uint32_t cur_record_no = 0;
        uint32_t cur_base_index = 0;
        uint32_t next_base_index = 0;
        for (iter = data.begin(); iter != data.end(); iter++)
        {
            // get next base index
            if (i >= 2)
            {
                next_base_index = m_reader_block[i]->GetNextBaseIndex(
                                      cur_record_no, cur_base_index);
            }

            // check key
            std::string cur_string;
            std::string test_key((*iter).key->data, (*iter).key->len);
            m_reader_block[i]->GetKeyByRecordNo(
                cur_record_no, cur_base_index, &cur_string);
            ASSERT_EQ(cur_string, test_key);

            buffer_len = 1024 * 10;
            ASSERT_EQ(sstable::kRetOK, m_reader_block[i]->GetKeyByRecordNo(
                          cur_record_no, cur_base_index, buffer, &buffer_len));
            cur_string = std::string(buffer, buffer_len);
            ASSERT_EQ(cur_string, test_key);

            cur_base_index = next_base_index;
            cur_record_no++;
        }
    }
}

// 测试GetValByRecordNo GetValPtrByRecordNo
TEST_F(ReaderBlockTest, GetValByRecordNo)
{
    // 初始化m_reader_block
    InitReaderBlock();
    char buffer[1024 * 10];
    uint32_t buffer_len = 1024 * 10;

    PrefixCompressedKey pre_fix_comp_key;
    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();

        test_set_it iter;
        uint32_t cur_record_no = 0;
        for (iter = data.begin(); iter != data.end(); iter++)
        {
            // check value
            std::string cur_string;
            std::string test_value((*iter).value->data, (*iter).value->len);
            m_reader_block[i]->GetValByRecordNo(
                cur_record_no, &cur_string);
            ASSERT_EQ(cur_string, test_value);

            buffer_len = 1024 * 10;
            ASSERT_EQ(sstable::kRetOK, m_reader_block[i]->GetValByRecordNo(
                          cur_record_no, buffer, &buffer_len));
            cur_string = std::string(buffer, buffer_len);
            ASSERT_EQ(cur_string, test_value);

            // check valPtr
            StringPiece val_piece;
            m_reader_block[i]->GetValPtrByRecordNo(
                cur_record_no, &val_piece);
            val_piece.copy_to_string(&cur_string);
            ASSERT_EQ(cur_string, test_value);

            buffer_len = 1024 * 10;
            char* val_ptr = NULL;
            m_reader_block[i]->GetValPtrByRecordNo(
                cur_record_no, &val_ptr, &buffer_len);
            cur_string = std::string(val_ptr, buffer_len);
            ASSERT_EQ(cur_string, test_value);

            cur_record_no++;
        }
    }
}


// 测试GetKVByRecordNo
TEST_F(ReaderBlockTest, GetKVByRecordNo)
{
     // 初始化m_reader_block
    InitReaderBlock();
    char key[1024 * 10];
    uint16_t key_len = 1024 * 10;
    char value[1024 * 10];
    uint32_t value_len = 1024 * 10;

    PrefixCompressedKey pre_fix_comp_key;
    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();

        test_set_it iter;
        uint32_t cur_record_no = 0;
        uint32_t cur_base_index = 0;
        uint32_t next_base_index = 0;
        for (iter = data.begin(); iter != data.end(); iter++)
        {
            // get next base index
            if (i >= 2)
            {
                next_base_index = m_reader_block[i]->GetNextBaseIndex(
                                      cur_record_no, cur_base_index);
            }

            // check key value
            value_len = 1024 * 10;
            key_len = 1024 * 10;
            std::string test_key((*iter).key->data, (*iter).key->len);
            std::string test_value((*iter).value->data, (*iter).value->len);

            ASSERT_EQ(sstable::kRetOK, m_reader_block[i]->GetKVByRecordNo(
                          cur_record_no, cur_base_index, key, &key_len, value, &value_len));
            std::string cur_key(key, key_len);
            std::string cur_value(value, value_len);

            ASSERT_EQ(cur_key, test_key);
            ASSERT_EQ(cur_value, test_value);

            cur_base_index = next_base_index;
            cur_record_no++;
        }
    }
}

// 测试FindRecordNoByKey
TEST_F(ReaderBlockTest, FindRecordNoByKey)
{
    // 初始化m_reader_block
    InitReaderBlock();
    char key[1024 * 10];
    uint16_t key_len = 1024 * 10;

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();

        test_set_it iter;
        uint32_t record_no = 0;
        uint32_t base_index = 0;
        bool is_accurated_hit = true;

        uint32_t first_same_key_no = 0;
        uint32_t cur_record_no = 0;
        std::string first_same_key;
        char lowbound_key[1024 * 10];  // 构造不存在的key，进行lowbound测试
        uint16_t lowbound_len = 0;

        for (iter = data.begin(); iter != data.end(); iter++)
        {
            std::string test_key((*iter).key->data, (*iter).key->len);

            if (cur_record_no == 0)
            {
                first_same_key = test_key;
            }
            else if (test_key != first_same_key)
            {
                lowbound_len = first_same_key.length();
                memcpy(lowbound_key, first_same_key.c_str(), lowbound_len);
                lowbound_key[lowbound_len - 1] += 1;

                first_same_key = test_key;
                first_same_key_no = cur_record_no;

                // lowbound查找
                ASSERT_EQ(sstable::kRetOK, m_reader_block[i]->FindRecordNoByKey(
                              lowbound_key, lowbound_len, &record_no,
                              &base_index, ReaderBlock::kLowBoundFind, &is_accurated_hit));

                ASSERT_EQ(cur_record_no, record_no);

                key_len = 1024 * 10;
                ASSERT_EQ(sstable::kRetOK, m_reader_block[i]->GetKeyByRecordNo(
                              record_no, base_index, key, &key_len));
                std::string cur_string = std::string(key, key_len);
                ASSERT_EQ(cur_string, test_key);
            }

            // 精确查找
            ASSERT_EQ(sstable::kRetOK, m_reader_block[i]->FindRecordNoByKey(
                          (*iter).key->data, (*iter).key->len, &record_no,
                          &base_index, ReaderBlock::kAccurateFind, &is_accurated_hit));

            ASSERT_EQ(first_same_key_no, record_no);

            key_len = 1024 * 10;
            ASSERT_EQ(sstable::kRetOK, m_reader_block[i]->GetKeyByRecordNo(
                          record_no, base_index, key, &key_len));
            std::string cur_string(key, key_len);
            ASSERT_EQ(cur_string, test_key);

            ++cur_record_no;
        }
    }
}
}  // namespace sstable

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
        // g_seed = 1731283778;

        printf("\nurl_num = %d, record_num = %d, seed = %d\n", g_url_num,
               g_record_num, g_seed);
        printf("threshold_len = %d, threshold_diff = %d\n\n", g_threshold_len,
               g_threshold_diff);
    }

    return RUN_ALL_TESTS();
}

