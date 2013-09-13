/*
 * =====================================================================================
 *
 *       Filename:  sstable_writer_test.cc
 *
 *    Description:  test for sstable_writer
 *
 *        Version:  1.0
 *        Created:  01/07/2011 11:20:00 AM
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

uint32_t g_url_num = 10; // 从文件中读取的url的数目
uint32_t g_record_num = 1000; // 用读取的url生成的测试数据的数目
uint32_t g_seed = 2222; // 随机种子
uint32_t g_block_size = 2 * 1024; // block的大小
uint32_t g_max_records = g_record_num;
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

class WriterTest: public testing::Test
{
public:
    WriterTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            m_table[i] = NULL;
        }
    }

    virtual ~WriterTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            delete m_table[i];
        }
    }

protected:

    virtual void SetUp()
    {
        RAW_LOG(INFO, "WriterTest SetUp");
    }

    virtual void TearDown()
    {
        RAW_LOG(INFO, "WriterTest TearDown");
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
        m_fixed_key_len = 10;
        m_fixed_value_len = 10;

        // fixed key, value
        m_table[0]= CreateFKFVTable(m_fixed_key_len, m_fixed_value_len, g_block_size,
                                    intern::BlockCompressionCodec::LZO);
        m_test_helper[0].Init("url.dat", g_url_num, g_record_num, g_seed,
                                    m_fixed_key_len, m_fixed_value_len);

        // fixed key, var value
        m_table[1]= CreateFKVVTable(m_fixed_key_len, g_block_size,
                                    intern::BlockCompressionCodec::LZO);
        m_test_helper[1].Init("url.dat", g_url_num, g_record_num, g_seed, m_fixed_key_len);

        // var key, fixed value
        m_table[2]= CreateVKFVTable(m_fixed_value_len, g_block_size,
                                    intern::BlockCompressionCodec::LZO);
        m_test_helper[2].Init("url.dat", g_url_num, g_record_num, g_seed, 0, m_fixed_value_len);

        // var key, var value
        m_table[3]= CreateVKVVTable(g_block_size,
                                    intern::BlockCompressionCodec::LZO);
        m_test_helper[3].Init("url.dat", g_url_num, g_record_num, g_seed);
    }

public:

    uint32_t m_fixed_key_len;       // fixed key length
    uint32_t m_fixed_value_len;     // fixed value length

    SSTableWriter* m_table[4];      // 四种测试block
    TestHelper m_test_helper[4];    // 用来测试四种block的测试数据
}; // class WriteTest

TEST_F(WriterTest, Open_Close)
{
    InitTestData();
    for (int i = 0; i < 4; ++i)
    {
        m_table[i]->Open();

        test_set_t& records = m_test_helper[i].GetTestData();
        WritePBRecordsToTable(records, m_table[i]);

        m_table[i]->Close();
    }
}

/*
TEST_F(WriterTest, CheckRecordValid)
{
    // 1. FK
    InitTestData();
    m_table[0]->Open();

    ASSERT_TRUE(m_table[0]->CheckRecordValid("1234567890", 10));
    ASSERT_EQ(kRetOK, m_table[0]->WriteRecord("1234567893", 10,
                                              "1234567890", 10));
    ASSERT_TRUE(m_table[0]->CheckRecordValid("1234567893", 10));
    ASSERT_FALSE(m_table[0]->CheckRecordValid("1234567892", 10));
    ASSERT_TRUE(m_table[0]->CheckRecordValid("1234567894", 10));

    m_table[0]->Close();

    // 2. VK
    m_table[2]->Open();

    ASSERT_TRUE(m_table[2]->CheckRecordValid("1234567890", 10));
    ASSERT_EQ(kRetOK, m_table[2]->WriteRecord("1234567893", 10,
                                              "1234567890", 10));
    ASSERT_TRUE(m_table[2]->CheckRecordValid("1234567893", 10));
    ASSERT_FALSE(m_table[2]->CheckRecordValid("1234567892", 10));
    ASSERT_FALSE(m_table[2]->CheckRecordValid("1234567893", 9));
    ASSERT_TRUE(m_table[2]->CheckRecordValid("12345678931", 11));
    ASSERT_TRUE(m_table[2]->CheckRecordValid("1234567894", 10));

    m_table[2]->Close();
}
*/

    // DumpXXX test in sstable_reader_test.cc

} // namespace sstable

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    google::InitGoogleLogging(argv[0]);
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
        // seed = 2087523071;
        printf("\nurl_num = %d, record_num = %d, seed = %d\n", g_url_num,
               g_record_num, g_seed);
        printf("threshold_len = %d, threshold_diff = %d\n\n", g_threshold_len,
               g_threshold_diff);
    }

    return RUN_ALL_TESTS();
}
