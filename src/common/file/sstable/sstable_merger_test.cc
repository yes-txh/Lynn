/*
 * =====================================================================================
 *
 *       Filename:  sstable_merger_test.cc
 *
 *    Description:  test for sstable_merger.cc
 *
 *        Version:  1.0
 *        Created:  01/14/2011 12:46:14 PM
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
#include "common/file/sstable/sstable_merger.h"
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

class SSTableMergerTest: public testing::Test
{
public:
    SSTableMergerTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            m_header[i] = NULL;
        }
    }

    virtual ~SSTableMergerTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            delete m_header[i];
        }
    }

protected:

    virtual void SetUp()
    {
        RAW_LOG(INFO, "SSTableMergerTest SetUp");
    }

    virtual void TearDown()
    {
        RAW_LOG(INFO, "SSTableMergerTest TearDown");
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

    // @brief 写入一组数据
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
            SSTableWriter writer_table(g_files_name[2][i].c_str(),
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
            SSTableWriter writer_table(g_files_name[3][i].c_str(),
                                       m_header[3]->options());

            ASSERT_EQ(sstable::kRetOK, writer_table.Open());
            test_set_t& records = m_test_helper[3].GetTestData();
            WritePBRecordsToTable(records, &writer_table);
            ASSERT_EQ(sstable::kRetOK, writer_table.Close());
        }
    }

public:

    uint32_t m_fixed_key_len;           // fixed key length
    uint32_t m_fixed_value_len;         // fixed value length

    SSTableHeader*  m_header[4];        // 四种测试block对应的block_header
    TestHelper m_test_helper[4];        // 用来测试四种block的测试数据
}; // class WriteTest

// 测试构造函数
TEST_F(SSTableMergerTest, Structure)
{
    int file_num = 10;
    InitTestData(file_num);

    std::string output_file_name[4];
    output_file_name[0] = std::string("fkfv_table");
    output_file_name[1] = std::string("fkvv_table");
    output_file_name[2] = std::string("vkfv_table");
    output_file_name[3] = std::string("vkvv_table");

    for (int i = 0; i < 4; ++i)
    {
        SSTableMerger merger2(g_files_name[i],
                              output_file_name[i],
                              m_header[i]->options(),
                              false);
    }

    for (int i = 0; i < 4; ++i)
    {
        std::vector<SSTableReader*> readers;
        for (int j = 0; j < file_num; ++j)
        {
            readers.push_back(new SSTableReader(g_files_name[i][j].c_str()));
            readers[j]->OpenFile();
        }

        SSTableMerger merger2(
            readers, output_file_name[i], m_header[i]->options());
        for (int j = 0; j < file_num; ++j)
        {
            readers[j]->CloseFile();
            delete readers[j];
        }
    }
}

// 测试多个文件合并
TEST_F(SSTableMergerTest, DoMerge)
{
    int file_num = 10;
    InitTestData(file_num);

    std::string output_file_name[4];
    output_file_name[0] = std::string("fkfv_table");
    output_file_name[1] = std::string("fkvv_table");
    output_file_name[2] = std::string("vkfv_table");
    output_file_name[3] = std::string("vkvv_table");

    // test for structure 1
    for (int i = 0; i < 4; ++i)
    {
        SSTableMerger merger(
            g_files_name[i], output_file_name[i], m_header[i]->options(), false);
        ASSERT_EQ(sstable::kRetOK, merger.DoMerge());
    }

    // test for structure 2
    for (int i = 0; i < 4; ++i)
    {
        std::vector<SSTableReader*> readers;
        for (int j = 0; j < file_num; ++j)
        {
            readers.push_back(new SSTableReader(g_files_name[i][j].c_str()));
            readers[j]->OpenFile();
        }

        SSTableMerger merger(
            readers, output_file_name[i], m_header[i]->options());
        ASSERT_EQ(sstable::kRetOK, merger.DoMerge());
        for (int j = 0; j < file_num; ++j)
        {
            readers[j]->CloseFile();
            delete readers[j];
        }
    }
}

// 验证多个文件合并后的结果是否正确
TEST_F(SSTableMergerTest, Check)
{
    int file_num = 10;
    InitTestData(file_num);

    std::string output_file_name[4];
    output_file_name[0] = std::string("fkfv_table");
    output_file_name[1] = std::string("fkvv_table");
    output_file_name[2] = std::string("vkfv_table");
    output_file_name[3] = std::string("vkvv_table");

    // test for structure 1
    for (int i = 0; i < 4; ++i)
    {
        SSTableMerger merger(
            g_files_name[i], output_file_name[i], m_header[i]->options(), false);
        ASSERT_EQ(sstable::kRetOK, merger.DoMerge());

        MultiSSTablesReader multi_reader(g_files_name[i]);
        SSTableReader reader(output_file_name[i].c_str());

        multi_reader.OpenFiles();
        reader.OpenFile();

        std::string multi_key;
        std::string key;
        std::string multi_value;
        std::string value;
        MultiSSTablesReader::Iterator* multi_iter = multi_reader.CreateIterator();
        SSTableReader::Iterator* iter = reader.CreateIterator();
        while ((!iter->Done()) && (!multi_iter->Done()))
        {
            iter->GetKVPair(&key, &value);
            multi_iter->GetKVPair(&multi_key, &multi_value);
            ASSERT_EQ(key, multi_key);
            ASSERT_EQ(value, multi_value);

            iter->Next();
            multi_iter->Next();
        }

        delete iter;
        delete multi_iter;

        multi_reader.CloseFiles();
        reader.CloseFile();
    }

    // test for structure 2
    for (int i = 0; i < 4; ++i)
    {
        std::vector<SSTableReader*> readers;
        for (int j = 0; j < file_num; ++j)
        {
            readers.push_back(new SSTableReader(g_files_name[i][j].c_str()));
            readers[j]->OpenFile();
        }

        SSTableMerger merger(
            readers, output_file_name[i], m_header[i]->options());
        ASSERT_EQ(sstable::kRetOK, merger.DoMerge());

        MultiSSTablesReader multi_reader(readers);
        SSTableReader reader(output_file_name[i].c_str());

        multi_reader.OpenFiles();
        reader.OpenFile();

        std::string multi_key;
        std::string key;
        std::string multi_value;
        std::string value;
        MultiSSTablesReader::Iterator* multi_iter = multi_reader.CreateIterator();
        SSTableReader::Iterator* iter = reader.CreateIterator();
        while ((!iter->Done()) && (!multi_iter->Done()))
        {
            iter->GetKVPair(&key, &value);
            multi_iter->GetKVPair(&multi_key, &multi_value);
            ASSERT_EQ(key, multi_key);
            ASSERT_EQ(value, multi_value);

            iter->Next();
            multi_iter->Next();
        }

        delete iter;
        delete multi_iter;

        multi_reader.CloseFiles();
        reader.CloseFile();

        for (int j = 0; j < file_num; ++j)
        {
            readers[j]->CloseFile();
            delete readers[j];
        }
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



