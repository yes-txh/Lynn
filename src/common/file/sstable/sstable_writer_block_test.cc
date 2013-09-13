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

#include "glog/logging.h"
#include "glog/raw_logging.h"
#include "common/base/byte_order.hpp"
#include "common/config/cflags.hpp"

#include "common/file/sstable/sstable_writer_block.h"
#include "common/file/sstable/sstable_reader_block.h"
#include "common/file/sstable/test_helper.h"

uint32_t g_url_num = 100;                   // 从文件中读取的url的数目
uint32_t g_record_num = 100;                // 用读取的url生成的测试数据的数目
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

class WriterBlockTest: public testing::Test
{
public:
    WriterBlockTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            m_header[i] = NULL;
            m_block[i] = NULL;
        }
    }

    virtual ~WriterBlockTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            delete m_header[i];
            delete m_block[i];
            m_test_helper[i].UnInit();
        }
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

    // @brief 生成4种测试用的数据，fkfv, fkvv, vkfv, vkvv;
    void InitTestData()
    {
        m_fixed_key_len = 20;
        m_fixed_value_len = 20;

        // fixed key, value
        m_block[0]= CreateFKFVBlock(m_fixed_key_len, m_fixed_value_len, g_block_size,
                                    intern::BlockCompressionCodec::LZO, m_header[0]);
        m_test_helper[0].Init("url.dat", g_url_num, g_record_num, g_seed, m_fixed_key_len,
                                    m_fixed_value_len);

        // fixed key, var value
        m_block[1]= CreateFKVVBlock(m_fixed_key_len, g_block_size,
                                    intern::BlockCompressionCodec::LZO, m_header[1]);
        m_test_helper[1].Init("url.dat", g_url_num, g_record_num, g_seed, m_fixed_key_len);

        // var key, fixed value
        m_block[2]= CreateVKFVBlock(m_fixed_value_len, g_block_size,
                                    intern::BlockCompressionCodec::LZO, m_header[2]);
        m_test_helper[2].Init("url.dat", g_url_num, g_record_num, g_seed, 0, m_fixed_value_len);

        // var key, var value
        m_block[3]= CreateVKVVBlock(g_block_size,
                                    intern::BlockCompressionCodec::LZO, m_header[3]);
        m_test_helper[3].Init("url.dat", g_url_num, g_record_num, g_seed);
    }

public:

    uint32_t m_fixed_key_len;       // fixed key length
    uint32_t m_fixed_value_len;     // fixed value length

    SSTableHeader*  m_header[4];    // 四种测试block对应的block_header
    WriterBlock* m_block[4];        // 四种测试block
    TestHelper m_test_helper[4];    // 用来测试四种block的测试数据
}; // class WriteBlockTest

// 测试ChangeBlockSize
TEST_F(WriterBlockTest, ChangeBlockSize)
{
    // init test data
    g_record_num = 10000; // 生成足够多的record, 使得现有的block_size装不下
    g_block_size = 20 * 1024;
    InitTestData();

    test_set_it iter;
    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        for (iter = data.begin(); iter != data.end(); iter++)
        {
            std::string key_string((*iter).key->data, (*iter).key->len);
            RetCode ret = m_block[i]->WriteRecord((*iter).key->data,
                                                  (*iter).key->len,
                                                  (*iter).value->data,
                                                  (*iter).value->len);
            if (ret == sstable::kRetBlockOverflow) // block溢出时，扩展block
            {
                // change the block size
                m_header[i]->mutable_options()->set_block_size(
                    m_header[i]->options().block_size() * 2);
                m_block[i]->ChangeBlockSize();

                // Write record again
                ret = m_block[i]->WriteRecord((*iter).key->data,
                                              (*iter).key->len,
                                              (*iter).value->data,
                                              (*iter).value->len);
            }

            ASSERT_EQ(ret, sstable::kRetOK);
        }
    }
}

// 测试Reset函数，将block写满数据，再Reset，重新写数据
TEST_F(WriterBlockTest, Reset)
{
    // init test data
    g_record_num = 100;
    g_block_size = 2 * 1024 * 1024;
    InitTestData();

    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        WritePBRecordsToBlock(data, m_block[i]);

        m_block[i]->Reset();

        WritePBRecordsToBlock(data, m_block[i]);
    }
}

// 测试GetLastKey,不断往block中插入数据，每插入一个数据便GetLastKey，验证是否正确
TEST_F(WriterBlockTest, GetLastKey)
{
    // init test data
    InitTestData();

    test_set_it iter;
    char buffer[2 * 1024 * 1024];
    uint32_t buffer_len = 0;
    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        for (iter = data.begin(); iter != data.end(); iter++)
        {
            std::string key_string((*iter).key->data, (*iter).key->len);
            ASSERT_EQ(sstable::kRetOK,
                      m_block[i]->WriteRecord((*iter).key->data,
                                              (*iter).key->len,
                                              (*iter).value->data,
                                              (*iter).value->len));
            // set m_last_record
            buffer_len = 2 * 1024 * 1024;
            ASSERT_EQ(kRetOK, m_block[i]->CompressBlockData(buffer, &buffer_len));
            // get m_last_record
            ASSERT_EQ(m_block[i]->GetLastKey(), key_string);
        }
    }
}

// 通过构造特殊数据来验证key是否应该成为BaseKey
TEST_F(WriterBlockTest, ShouldBeBase)
{
    InitTestData();

    // ShouldBeBase just called in the case that keylen is variant;
    for (int i = 2; i < 4; ++i)
    {
        m_block[i]->Reset();
    }

    uint16_t prefix_len = 0;
    for (int i = 2; i < 4; ++i)
    {
        std::string  cur_key = "cn.com.sina.www/abcd/1234.html";
        std::string  cur_val = "cn.com.sina.www/abcd/1234.html";
        prefix_len = 0;
        // the first record should be base;
        ASSERT_TRUE(m_block[i]->ShouldBeBase(cur_key.c_str(),
                                             cur_key.length(),
                                             &prefix_len));
        ASSERT_EQ(prefix_len, 0);

        // write a record;
        ASSERT_EQ(sstable::kRetOK, m_block[i]->WriteVarKeyLenRecord(cur_key.c_str(),
                  cur_key.length(),
                  cur_val.c_str(),
                  m_fixed_value_len));

        // test for return true;
        prefix_len = 0;
        cur_key = "cn.net.sina.www/abcd/1234.html";
        ASSERT_TRUE(m_block[i]->ShouldBeBase(cur_key.c_str(),
                                             cur_key.length(),
                                             &prefix_len));
        ASSERT_EQ(prefix_len, 0);

        // test for return false;
        prefix_len = 0;
        cur_key = "cn.com.sina.www/accdefg/1234.html";
        ASSERT_FALSE(m_block[i]->ShouldBeBase(cur_key.c_str(),
                                              cur_key.length(),
                                              &prefix_len));
        ASSERT_EQ(prefix_len, 17);

        // write a record to guarantee that prev_key is not a base_key;
        ASSERT_EQ(sstable::kRetOK, m_block[i]->WriteVarKeyLenRecord(cur_key.c_str(),
                  cur_key.length(),
                  cur_val.c_str(),
                  m_fixed_value_len));

        // test for return true
        prefix_len = 0;
        cur_key = "cn.net.sina.www/accdefg/1234.html";
        ASSERT_TRUE(m_block[i]->ShouldBeBase(cur_key.c_str(),
                                             cur_key.length(),
                                             &prefix_len));
        ASSERT_EQ(prefix_len, 0);

        // test for return false
        prefix_len = 0;
        cur_key = "cn.com.sina.www/adcdefg/1235.html";
        ASSERT_FALSE(m_block[i]->ShouldBeBase(cur_key.c_str(),
                                              cur_key.length(),
                                              &prefix_len));
        ASSERT_EQ(prefix_len, 17);
    }
}

// 对CompressBlockData进行测试,更详细的测试需要结合sstable_reader_block进行
TEST_F(WriterBlockTest, CompressBlockData)
{
    InitTestData();
    uint32_t buffer_len = 2 * 1024 * 1024;
    char buffer[2 * 1024 * 1024];

    for (int i = 0; i < 4; ++i)
    {
        // 往m_block中添加数据
        test_set_t& data = m_test_helper[i].GetTestData();
        WritePBRecordsToBlock(data, m_block[i]);

        // 压缩WriterBlock中的数据
        buffer_len = g_block_size;
        ASSERT_EQ(sstable::kRetOK,
                  m_block[i]->CompressBlockData(buffer, &buffer_len));

        // 解压缩数据到ReaderBlock中
        ReaderBlock* reader_block = new ReaderBlock(m_header[i]);
        ASSERT_EQ(sstable::kRetOK,
                  reader_block->UnCompressBlockData(buffer, buffer_len));
    }
}

//  对4种kv值测试WriteRecord
TEST_F(WriterBlockTest, WriteRecord)
{
    g_record_num = 100;
    g_block_size = 2 * 1024 * 1024;
    InitTestData();

    test_set_it iter;
    for (int i = 0; i < 4; ++i)
    {
        test_set_t& data = m_test_helper[i].GetTestData();
        for (iter = data.begin(); iter != data.end(); iter++)
        {
            std::string key_string((*iter).key->data, (*iter).key->len);
            ASSERT_EQ(sstable::kRetOK,
                      m_block[i]->WriteRecord((*iter).key->data,
                                              (*iter).key->len,
                                              (*iter).value->data,
                                              (*iter).value->len));
        }
    }
}

//  对FKFV和FKVV测试WriteRecord
TEST_F(WriterBlockTest, WriteFixedKeyLenRecord)
{
    g_record_num = 10;
    g_block_size = 256;
    InitTestData();

    // 1.FKFV
    test_set_it iter;
    uint32_t i = 0;
    test_set_t& data = m_test_helper[0].GetTestData();
    for (iter = data.begin(), i = 0; iter != data.end(); iter++, i++)
    {
        std::string key_string((*iter).key->data, (*iter).key->len);
        if (i> 3)
        {
            // ret ok
            ASSERT_EQ(kRetBlockOverflow,
                      m_block[0]->WriteRecord((*iter).key->data,
                                              (*iter).key->len,
                                              (*iter).value->data,
                                              (*iter).value->len));
        }
        else
        {
            // block over flow
            ASSERT_EQ(kRetOK,
                      m_block[0]->WriteRecord((*iter).key->data,
                                              (*iter).key->len,
                                              (*iter).value->data,
                                              (*iter).value->len));
        }
    }

    // 2.FKVV
    g_block_size = 2 * 1024 * 1024;
    InitTestData();

    data = m_test_helper[1].GetTestData();
    for (iter = data.begin(), i = 0; iter != data.end(); iter++, i++)
    {
        std::string key_string((*iter).key->data, (*iter).key->len);
        if (i> 2)
            break;
        else
        {
            // block over flow
            ASSERT_EQ(kRetOK,
                      m_block[1]->WriteRecord((*iter).key->data,
                                              (*iter).key->len,
                                              (*iter).value->data,
                                              (*iter).value->len));
        }
    }

    // ret block over flow
    m_header[1]->mutable_options()->set_block_size(
        m_block[1]->GetWrittenBytes());
    m_block[1]->ChangeBlockSize();
    ASSERT_EQ(kRetBlockOverflow,
              m_block[1]->WriteRecord("key_test",
                                      20,
                                      "val_test",
                                      strlen("val_test")));
}

//  对VKFV和VKVV测试WriteRecord, 分basekey和非basekey
//  5会core,原因待查
TEST_F(WriterBlockTest, WriteVarKeyLenRecord)
{
    g_record_num = 100;
    g_block_size = 2 * 1024 * 1024;
    InitTestData();

    test_set_it iter;
    uint32_t i = 0;
    test_set_t& data = m_test_helper[2].GetTestData();
    // 3.VKFV
    for (iter = data.begin(), i = 0; iter != data.end(); iter++, i++)
    {
        std::string key_string((*iter).key->data, (*iter).key->len);
        ASSERT_EQ(kRetOK, m_block[2]->WriteRecord((*iter).key->data,
                  (*iter).key->len,
                  (*iter).value->data,
                  (*iter).value->len));
    }

    m_block[2]->Reset();
    m_header[2]->mutable_options()->set_block_size(120);
    m_block[2]->ChangeBlockSize();

    const char* key1 = "abcdefg.1234567";
    uint16_t key_len = strlen(key1);
    const char* val  = "abcdefghhhhhhhhhhhhhhhhhhhh";
    uint32_t val_len = 20;
    ASSERT_EQ(kRetOK, m_block[2]->WriteRecord(key1, key_len, val, val_len));

    // 3.2 should be basekey
    // 3.2.1 ok
    char* key2 = const_cast<char*>("abcedfg.1234567");
    key_len    = strlen(key2);
    ASSERT_EQ(kRetBlockOverflow, m_block[2]->WriteRecord(key2, key_len, val, val_len));

    // 3.2.2 block over flow
    key2 = const_cast<char*>("abcdefg.1234567");
    key_len    = strlen(key2);
    ASSERT_EQ(kRetBlockOverflow, m_block[2]->WriteRecord(key2, key_len, val, val_len));

    // 3.3 should not be basekey
    // 3.3.1 ok
    m_header[2]->mutable_options()->set_block_size(150);
    m_block[2]->ChangeBlockSize();
    key2      = const_cast<char*>("abcedfg*1234567");
    key_len    = strlen(key2);
    ASSERT_EQ(kRetOK, m_block[2]->WriteRecord(key2, key_len, val, val_len));

    // 3.3.2 block over flow
    ASSERT_EQ(kRetBlockOverflow, m_block[2]->WriteRecord(key2, key_len, val, val_len));

    // 4.VKVV

    // 4.1 random key
    data = m_test_helper[3].GetTestData();
    for (iter = data.begin(), i = 0; iter != data.end(); iter++, i++)
    {
        std::string key_string((*iter).key->data, (*iter).key->len);
        ASSERT_EQ(kRetOK, m_block[3]->WriteRecord((*iter).key->data,
                  (*iter).key->len,
                  (*iter).value->data,
                  (*iter).value->len));
    }

    m_block[3]->Reset();
    m_header[3]->mutable_options()->set_block_size(170);
    m_block[3]->ChangeBlockSize();
    m_block[3]->WriteRecord(key1, key_len, val, val_len);

    // 4.2 should be basekey
    // 4.2.1 ok
    key2 = const_cast<char*>("abcedfg.1234567");
    key_len    = strlen(key2);
    ASSERT_EQ(kRetOK, m_block[3]->WriteRecord(key2, key_len, val, val_len));

    // 4.2.2 block over flow
    key2 = const_cast<char*>("abcdefg.1234567");
    key_len    = strlen(key2);
    ASSERT_EQ(kRetBlockOverflow, m_block[3]->WriteRecord(key2, key_len, val, val_len));

    // 4.3 should not be basekey
    // 4.3.1 ok
    m_header[3]->mutable_options()->set_block_size(210);
    m_block[3]->ChangeBlockSize();
    key2      = const_cast<char*>("abcedfg*1234567");
    key_len    = strlen(key2);
    ASSERT_EQ(kRetOK, m_block[3]->WriteRecord(key2, key_len, val, val_len));

    // 4.3.2 block over flow
    ASSERT_EQ(kRetBlockOverflow, m_block[3]->WriteRecord(key2, key_len, val, val_len));

    // 5 core...unsolve
    m_block[3]->Reset();
    m_header[3]->mutable_options()->set_block_size(2 * 1024 * 1024);
    m_block[3]->ChangeBlockSize();
    m_block[3]->WriteRecord(key1, key_len, val, val_len);

    m_header[3]->mutable_options()->set_block_size(m_block[3]->GetWrittenBytes());
    m_block[3]->ChangeBlockSize();
    key2 = const_cast<char*>("abcedfg.1234567");
    key_len    = strlen(key2);
    ASSERT_EQ(kRetBlockOverflow, m_block[3]->WriteRecord(key2, key_len, val, val_len));
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
        g_seed = 2087523071;
        printf("\nurl_num = %d, record_num = %d, seed = %d\n", g_url_num,
               g_record_num, g_seed);
        printf("threshold_len = %d, threshold_diff = %d\n\n", g_threshold_len,
               g_threshold_diff);
    }

    return RUN_ALL_TESTS();
}

