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

    // 构造和析构
    explicit SSTableWriter(const char* file_path,
                           const SSTableOptions& options);
    virtual ~SSTableWriter();

    /// @brief     同步写打开一个sstable 文件, 非线程安全
    /// @param     file_path, 文件的路径;
    /// @retval    kRetOK:      成功
    ///            kRetIOError: 打开文件失败
    RetCode        Open();

    /// @brief     同步关闭文件, 非线程安全
    ///            关闭文件时会将BloomFilter,
    ///            index block以及schema写入到指定的文件当中;
    /// @param     无
    /// @retval    kRetOK:  成功
    ///            kRetIOError: 关闭文件失败
    RetCode        Close();

    /// @brief      同步向SSTable当中写入一条记录, 非线程安全
    ///              当当前块写满时需要将当前块Dump到文件中，同时使用新的块
    /// @param      key, 写入记录的key
    /// @param      key_len, 写入记录的key len
    /// @param      val, 写入记录的val
    /// @param      val_len, 写入记录的val len
    /// @retval     kRetOK:         写入成功
    ///             kCompressError: 压缩错误
    ///             kRetFileOverflow: 文件写满
    ///             kRetParamError: 参数错误, 单条记录大小超过块大小
    ///             kRetIOError: IO错误;
    RetCode         WriteRecord(const void* key, uint16_t key_len,
                                const void* val, uint32_t val_len);

    /// @brief      同步向SSTable当中写入一条记录, 非线程安全
    ///              当当前块写满时需要将当前块Dump到文件中，同时使用新的块
    /// @param      key, 写入记录的key
    /// @param      val, 写入记录的val
    /// @retval     kRetOK:         写入成功
    ///             kCompressError: 压缩错误
    ///             kRetFileOverflow: 文件写满
    ///             kRetParamError: 参数错误, 单条记录大小超过块大小
    ///             kRetIOError: IO错误;
    inline RetCode  WriteRecord(const std::string& key, const std::string& val)
    {
        assert(key.length() <= kMaxKeyLen);
        return WriteRecord(key.data(), key.length(), val.data(), val.length());
    }

private:

    /// 缺省512K的index block size,写入不够时需要进行扩充;
    static const  uint32_t kDefaultIndexBlockSize = 512 * 1024;

    /// 缺省512K的compress buffer size,写入不够时需要进行扩充;
    static const  uint32_t kDefaultCompressBufferSize = 512 * 1024;

    /// 默认write buffer的长度
    static const  uint32_t kDefaultWriteBufferSize = 512 * 1024;

    /// @brief    保存当前block的索引;
    void ReserveIndex();

    /// @brief    检测写入的record的合法性;
    /// @param    key, key的指针;
    /// @param    key_len, key的长度;
    /// @retval   true 合法，false, 不合法;
    bool CheckRecordValid(const void* key, uint16_t key_len);

    /// @brief    向文件当中dump datablock;
    /// @param
    /// @retval   kRetOK, 成功;
    ///           kRetFileOverflow, 文件已经写满,不能再写入;
    ///           kRetIOError, IO错误;
    ///           kUncompressError
    RetCode       DumpCurDataBlock();

    /// @brief    向文件当中写入Bloomfilter;
    /// @param    无
    /// @retval   kRetOk: 成功
    ///           kRetIOError: dump bloom filter失败
    RetCode       DumpBloomfilter();

    /// @brief    向文件当中写入Index;
    /// @param    无
    /// @retval   kRetOK:  成功
    ///           kRetIOError: dump index失败
    RetCode       DumpIndex();

    /// @brief    向文件当中写入sstable的头部信息;
    /// @param    无
    /// @retval   kRetOK: 成功
    ///           kRetIOError:  dump schema失败
    RetCode       DumpSSTableHeader();

    File*           m_file;                ///< file对象的指针;
    std::string     m_file_name;           ///< 文件名;
    SSTableHeader   m_sstable_header;      ///< sstable的schema;
    uint32_t        m_written_num_records; ///< 已经写入的记录数目;
    uint16_t        m_written_num_blocks;  ///< 已经写入文件中的块数目;

    char*           m_compress_buffer;     ///< 写文件的Buffer;
    uint32_t        m_compress_buffer_size; ///< Buffer的Size;
    uint32_t        m_compress_len;         ///< 当前块压缩之后的长度;

    char*           m_write_buffer;       ///< 写的Buffer;
    uint64_t        m_write_buffer_size;  ///< 写的Buffer的长度;
    uint64_t        m_write_buffer_offset;///< 写的Buffer的Offset;
    uint64_t        m_total_data_bytes;   ///< 已写入block压缩前的总长度
    uint64_t        m_total_compressed_bytes;  ///< 已写入block压缩后的总长度

    BloomFilter*    m_bloomfilter;        ///< 用于保存bloomfilter对象的指针;

    SSTableHeader   m_index_description;  ///< index块的Schema;
    WriterBlock*    m_index_block;        ///< 用于写的index block;
    WriterBlock*    m_cur_data_block;     ///< 用于写的当前block;
    std::string     m_prev_record;        ///< 上一条记录的key;
};

} // namespace sstable

#endif // SSTABLE_SSTABLE_WRITER_H_
