#ifndef SSTABLE_SSTABLE_WRITER_H_
#define SSTABLE_SSTABLE_WRITER_H_

#include <string>

#include "common/base/stdint.h"
#include "common/collection/bloom_filter.hpp"

#include "common/file/sstable/sstable_def.h"
#include "common/file/sstable/sstable_header.pb.h"

#include "common/file/file.h"
#include "thirdparty/gtest/gtest.h"

namespace sstable
{
class WriterBlock;

class SSTableWriter
{
public:

    // friend for test
    friend class WriterTest;
    friend class ReaderTest;
    FRIEND_TEST(WriterTest, CheckRecordValid);
    FRIEND_TEST(ReaderTest, FindFirstValue);

    // ���������
    explicit SSTableWriter(const char* file_path,
                           const SSTableOptions& options);
    virtual ~SSTableWriter();

    /// @brief     ͬ��д��һ��sstable �ļ�, ���̰߳�ȫ
    /// @param     file_path, �ļ���·��;
    /// @retval    kRetOK:      �ɹ�
    ///            kRetIOError: ���ļ�ʧ��
    RetCode        Open();

    /// @brief     ͬ���ر��ļ�, ���̰߳�ȫ
    ///            �ر��ļ�ʱ�ὫBloomFilter,
    ///            index block�Լ�schemaд�뵽ָ�����ļ�����;
    /// @param     ��
    /// @retval    kRetOK:  �ɹ�
    ///            kRetIOError: �ر��ļ�ʧ��
    RetCode        Close();

    /// @brief      ͬ����SSTable����д��һ����¼, ���̰߳�ȫ
    ///              ����ǰ��д��ʱ��Ҫ����ǰ��Dump���ļ��У�ͬʱʹ���µĿ�
    /// @param      key, д���¼��key
    /// @param      key_len, д���¼��key len
    /// @param      val, д���¼��val
    /// @param      val_len, д���¼��val len
    /// @retval     kRetOK:         д��ɹ�
    ///             kCompressError: ѹ������
    ///             kRetFileOverflow: �ļ�д��
    ///             kRetParamError: ��������, ������¼��С�������С
    ///             kRetIOError: IO����;
    RetCode         WriteRecord(const void* key, uint16_t key_len,
                                const void* val, uint32_t val_len);

    /// @brief      ͬ����SSTable����д��һ����¼, ���̰߳�ȫ
    ///              ����ǰ��д��ʱ��Ҫ����ǰ��Dump���ļ��У�ͬʱʹ���µĿ�
    /// @param      key, д���¼��key
    /// @param      val, д���¼��val
    /// @retval     kRetOK:         д��ɹ�
    ///             kCompressError: ѹ������
    ///             kRetFileOverflow: �ļ�д��
    ///             kRetParamError: ��������, ������¼��С�������С
    ///             kRetIOError: IO����;
    inline RetCode  WriteRecord(const std::string& key, const std::string& val)
    {
        assert(key.length() <= kMaxKeyLen);
        return WriteRecord(key.data(), key.length(), val.data(), val.length());
    }

private:

    /// ȱʡ512K��index block size,д�벻��ʱ��Ҫ��������;
    static const  uint32_t kDefaultIndexBlockSize = 512 * 1024;

    /// ȱʡ512K��compress buffer size,д�벻��ʱ��Ҫ��������;
    static const  uint32_t kDefaultCompressBufferSize = 512 * 1024;

    /// Ĭ��write buffer�ĳ���
    static const  uint32_t kDefaultWriteBufferSize = 512 * 1024;

    /// @brief    ���浱ǰblock������;
    void ReserveIndex();

    /// @brief    ���д���record�ĺϷ���;
    /// @param    key, key��ָ��;
    /// @param    key_len, key�ĳ���;
    /// @retval   true �Ϸ���false, ���Ϸ�;
    bool CheckRecordValid(const void* key, uint16_t key_len);

    /// @brief    ���ļ�����dump datablock;
    /// @param
    /// @retval   kRetOK, �ɹ�;
    ///           kRetFileOverflow, �ļ��Ѿ�д��,������д��;
    ///           kRetIOError, IO����;
    ///           kUncompressError
    RetCode       DumpCurDataBlock();

    /// @brief    ���ļ�����д��Bloomfilter;
    /// @param    ��
    /// @retval   kRetOk: �ɹ�
    ///           kRetIOError: dump bloom filterʧ��
    RetCode       DumpBloomfilter();

    /// @brief    ���ļ�����д��Index;
    /// @param    ��
    /// @retval   kRetOK:  �ɹ�
    ///           kRetIOError: dump indexʧ��
    RetCode       DumpIndex();

    /// @brief    ���ļ�����д��sstable��ͷ����Ϣ;
    /// @param    ��
    /// @retval   kRetOK: �ɹ�
    ///           kRetIOError:  dump schemaʧ��
    RetCode       DumpSSTableHeader();

    File*           m_file;                ///< file�����ָ��;
    std::string     m_file_name;           ///< �ļ���;
    SSTableHeader   m_sstable_header;      ///< sstable��schema;
    uint32_t        m_written_num_records; ///< �Ѿ�д��ļ�¼��Ŀ;
    uint16_t        m_written_num_blocks;  ///< �Ѿ�д���ļ��еĿ���Ŀ;

    char*           m_compress_buffer;     ///< д�ļ���Buffer;
    uint32_t        m_compress_buffer_size; ///< Buffer��Size;
    uint32_t        m_compress_len;         ///< ��ǰ��ѹ��֮��ĳ���;

    char*           m_write_buffer;       ///< д��Buffer;
    uint64_t        m_write_buffer_size;  ///< д��Buffer�ĳ���;
    uint64_t        m_write_buffer_offset;///< д��Buffer��Offset;
    uint64_t        m_total_data_bytes;   ///< ��д��blockѹ��ǰ���ܳ���
    uint64_t        m_total_compressed_bytes;  ///< ��д��blockѹ������ܳ���

    BloomFilter*    m_bloomfilter;        ///< ���ڱ���bloomfilter�����ָ��;

    SSTableHeader   m_index_description;  ///< index���Schema;
    WriterBlock*    m_index_block;        ///< ����д��index block;
    WriterBlock*    m_cur_data_block;     ///< ����д�ĵ�ǰblock;
    std::string     m_prev_record;        ///< ��һ����¼��key;
};

} // namespace sstable

#endif // SSTABLE_SSTABLE_WRITER_H_
