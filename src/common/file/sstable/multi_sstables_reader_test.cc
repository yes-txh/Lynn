/*
 * =====================================================================================
 *
 *       Filename:  multi_sstables_reader_test.cc
 *
 *    Description:  test for multi_sstables_reader.cc
 *
 *        Version:  1.0
 *        Created:  01/13/2011 01:33:15 PM
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
#include "common/file/sstable/test_helper.h"
#include "common/file/sstable/multi_sstables_reader.h"
#include "common/file/sstable/sstable_writer.h"

uint32_t g_url_num = 10;                // ���ļ��ж�ȡ��url����Ŀ
uint32_t g_record_num = 1000;           // �ö�ȡ��url���ɵĲ������ݵ���Ŀ
uint32_t g_seed = 2222;                 // �������
uint32_t g_block_size = 2 * 1024;       // block�Ĵ�С
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

class MultiReaderTest: public testing::Test
{
public:
    MultiReaderTest()
    {
        for (int i = 0; i < 4; ++i)
        {
            m_header[i] = NULL;
            m_multi_reader_table[i] = NULL;
        }
    }

    virtual ~MultiReaderTest()
    {
    }

protected:

    virtual void SetUp()
    {
        RAW_LOG(INFO, "MultiReaderTest SetUp");
    }

    virtual void TearDown()
    {
        RAW_LOG(INFO, "MultiReaderTest TearDown");
    }

    // @brief ��ʼ��һ������key����ֵ��д���
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

    // @brief ��ʼ��һ������Key�䳤ֵ��д���
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

    // @brief ��ʼ��һ���䳤Key����ֵ��д���
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

    // @brief ��ʼ��һ���䳤Key����ֵ��д���
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

    // @brief д��һ�����ݵ�Ŀ��;
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


    // @brief ����4�ֲ����õ����ݣ�fkfv, fkvv, vkfv, vkvv;
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
        delete m_header[0];


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
        delete m_header[1];

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
        delete m_header[2];

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
        delete m_header[3];
    }

public:

    uint32_t m_fixed_key_len;           // fixed key length
    uint32_t m_fixed_value_len;         // fixed value length

    SSTableHeader*  m_header[4];        // ���ֲ���block��Ӧ��block_header
    MultiSSTablesReader* m_multi_reader_table[4];   // ���ֲ���reader_block
    TestHelper m_test_helper[4];        // ������������block�Ĳ�������
}; // class MultiReaderTest

// ����Open,Close
TEST_F(MultiReaderTest, Open_Writer_Close)
{
    int file_num = 10;

    InitTestData(file_num);

    for (int i = 0; i < 4; ++i)
    {
        m_multi_reader_table[i] = new MultiSSTablesReader(g_files_name[i]);
        ASSERT_EQ(sstable::kRetOK, m_multi_reader_table[i]->OpenFiles());
        ASSERT_EQ(sstable::kRetOK, m_multi_reader_table[i]->CloseFiles());
        delete m_multi_reader_table[i];
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