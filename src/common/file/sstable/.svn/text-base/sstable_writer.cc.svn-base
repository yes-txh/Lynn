#include "common/base/stdint.h"
#include "common/base/byte_order.hpp"
#include "common/file/sstable/sstable_writer.h"
#include "common/file/sstable/sstable_writer_block.h"
#include "common/crypto/hash/md5.hpp"
#include "thirdparty/glog/logging.h"

namespace sstable
{
SSTableWriter::SSTableWriter(const char* file_name,
                             const SSTableOptions& options):
    m_file(NULL),
    m_file_name(file_name),
    m_written_num_records(0),
    m_written_num_blocks(0),
    m_compress_buffer(NULL),
    m_compress_buffer_size(0),
    m_compress_len(0),
    m_write_buffer(NULL),
    m_write_buffer_size(0),
    m_write_buffer_offset(0),
    m_total_data_bytes(0),
    m_total_compressed_bytes(0),
    m_bloomfilter(NULL),
    m_index_block(NULL),
    m_cur_data_block(NULL)
{
    m_sstable_header.mutable_options()->CopyFrom(options);
    // 计算sstable file_id, 用于cache
    m_sstable_header.set_sstable_id(MD5::GetHash64(m_file_name));

    if (options.bloomfilter_size() > 0) {
        // 1:初始化bloomfilter的内存;
        m_bloomfilter = new BloomFilter(options.bloomfilter_size(),
                                        options.bloomfilter_prob());
    }

    // 2:初始化index_block的内存;
    m_index_description.mutable_options()->set_block_size(
        kDefaultIndexBlockSize);
    m_index_description.mutable_options()->set_compress_type(
        options.compress_type());
    m_index_description.mutable_options()->set_kv_type(
        (options.kv_type() & kKeyTypeMask) | (kTypeFixedLen << 4));
    m_index_description.mutable_options()->set_fixed_data_len(sizeof(uint64_t));

    if ((options.kv_type() & kKeyTypeMask) == kTypeFixedLen)
    {
        m_index_description.mutable_options()->set_fixed_key_len(
            options.fixed_key_len());
    }

    m_index_block = new WriterBlock(&m_index_description);

    // 3:初始化cur_dat_block的内存;
    m_cur_data_block = new WriterBlock(&m_sstable_header);

    // 4:分配压缩buffer
    m_compress_buffer = new char[kDefaultCompressBufferSize];
    m_compress_buffer_size = kDefaultCompressBufferSize;

    // 5:分配write buffer
    m_write_buffer = new char[kDefaultWriteBufferSize];
    m_write_buffer_size = kDefaultWriteBufferSize;
}

SSTableWriter::~SSTableWriter()
{
    if (m_file != NULL)
    {
        RetCode ret = Close();
        assert(ret == kRetOK);
        (void) ret;
    }

    delete m_bloomfilter;
    delete m_index_block;
    delete m_cur_data_block;
    delete[] m_compress_buffer;
    delete[] m_write_buffer;
}

/// @brief     同步写打开一个sstable 文件, 非线程安全
/// @param     file_path, 文件的路径;
/// @retval    kRetOK:      成功
///            kRetIOError: 打开文件失败
RetCode SSTableWriter::Open()
{
    RetCode ret = kRetOK;
    assert(m_file == NULL);

    VLOG(0) << " sstable writer open file: " << m_file_name.c_str();

    // 1:打开文件;
    m_file =::File::Open(m_file_name.c_str(), ::File::ENUM_FILE_OPEN_MODE_W);

    if (m_file == NULL)
    {
        LOG(ERROR) << " open failed, file name: " << m_file_name.c_str();
        ret = kRetIOError;
    }

    return ret;
}

/// @brief     同步关闭文件, 非线程安全
///            关闭文件时会将BloomFilter,
///            index block以及schema写入到指定的文件当中;
/// @param     无
/// @retval    kRetOK:  成功
///            kRetIOError: 关闭文件失败
///            kRetCompressError: 压缩block失败
///            kRetFileOverflow: 文件overflow
RetCode  SSTableWriter::Close()
{
    RetCode ret = kRetOK;
    VLOG(0) << " sstable writer close file: " << m_file_name.c_str();

    // Dump最后一个DataBlock;
    if (m_cur_data_block->GetNumRecords() > 0)
    {
        ret = DumpCurDataBlock();
        if (ret == kRetCompressError)
        {
            LOG(ERROR) << "sstable block compress error, file name: "
                       << m_file_name.c_str() << " error: " << ret;
            return kRetCompressError;
        }

        if (ret == kRetFileOverflow) return kRetFileOverflow;

        uint32_t last_size = m_compress_len + m_write_buffer_offset;
        if(last_size > m_write_buffer_size)
        {
            char* temp_buffer = new char[last_size];
            ::memcpy(temp_buffer, m_write_buffer, m_write_buffer_offset);
            ::memcpy(temp_buffer + m_write_buffer_offset, m_compress_buffer, m_compress_len);
            delete[] m_write_buffer;
            m_write_buffer = temp_buffer;
            m_write_buffer_size = last_size;
            m_write_buffer_offset = last_size;
        }
        else
        {
            ::memcpy(m_write_buffer + m_write_buffer_offset,m_compress_buffer, m_compress_len);
            m_write_buffer_offset += m_compress_len;
        }

        uint32_t error_code = 0;
        int64_t written_len = m_file->Write(m_write_buffer,
                                            m_write_buffer_offset,
                                            &error_code);
        if (error_code != ERR_FILE_OK)
        {
            LOG(ERROR) << "dump write buffer failed, write buffer bytes:"
                       << m_write_buffer_offset;
            assert(written_len == 0);
            (void) written_len;
            return kRetIOError;
        }

        // 设置header信息
        m_sstable_header.set_raw_length(m_total_data_bytes);
        m_sstable_header.set_compressed_len(m_total_compressed_bytes);
        m_write_buffer_offset = 0;

        ReserveIndex();
        m_written_num_blocks++;
    }

    assert(ret == kRetOK);
    ret = DumpBloomfilter();

    if (ret != kRetOK)
    {
        LOG(ERROR) << "dump bloomfilter failed, file name: " << m_file_name.c_str();
        return ret;
    }

    ret = DumpIndex();

    if (ret != kRetOK)
    {
        LOG(ERROR) << "dump index failed, file name: " << m_file_name.c_str();
        return ret;
    }

    ret = DumpSSTableHeader();

    if (ret != kRetOK)
    {
        LOG(ERROR) << "dump schema failed, file name: " << m_file_name.c_str();
        return ret;
    }

    uint32_t error_no = 0;
    int32_t close_ret = m_file->Close(&error_no);

    if (close_ret < 0)
    {
        LOG(ERROR) << "close file failed, file name: " << m_file_name.c_str();
        ret = kRetIOError;
    }
    else
    {
        VLOG(0) << "close file succeed, file name: " << m_file_name.c_str();
        m_file = NULL;
    }

    return ret;
}

/// @brief      同步向SSTable当中写入一条记录, 非线程安全
///              当当前块写满时需要将当前块Dump到文件中，同时使用新的块
/// @param      key, 写入记录的key
/// @param      key_len, 写入记录的key len
/// @param      val, 写入记录的data
/// @param      val_len, 写入记录的val len
/// @retval     kRetOK:         写入成功
///             kCompressError: 压缩错误
///             kRetFileOverflow: 文件写满
///             kRetParamError: 参数错误, 单条记录大小超过块大小
///             kRetIOError: IO错误;
RetCode SSTableWriter::WriteRecord(const void* key,
                                   uint16_t key_len,
                                   const void* val,
                                   uint32_t val_len)
{
    RetCode ret = kRetBlockOverflow;
    assert(m_cur_data_block != NULL);

    if (m_sstable_header.options().bloomfilter_size() > 0 &&
        m_written_num_records >= m_sstable_header.options().bloomfilter_size())
    {
        return kRetFileOverflow;
    }

    // 在Debug模式下检测record的合法性,
    assert(CheckRecordValid(key, key_len));

    do
    {
        // 1: 将数据写入到write block当中;
        ret = m_cur_data_block->WriteRecord(key,
                                            key_len,
                                            val,
                                            val_len);

        if (ret == kRetBlockOverflow)
        {
            if (m_cur_data_block->GetNumRecords() == 0)
            {
                // 一个record的大小超过了块的大小;
                ret = kRetParamError;
            }
            else
            {
                // 1: 当前块已经满,压缩当前块
                ret = DumpCurDataBlock();
                if (ret == kRetCompressError)
                {
                    LOG(ERROR) << "sstable block compress error, file name: "
                               << m_file_name << " error: " << ret;
                    return kRetCompressError;
                }

                if (ret == kRetFileOverflow) return kRetFileOverflow;

                if ((m_compress_len > m_write_buffer_size)
                    && (m_write_buffer_offset == 0))
                {
                    // 若块压缩之后的大小大于BufferSize并且Buffer为空时，需要扩充Buffer
                    delete[] m_write_buffer;
                    m_write_buffer = new char[m_compress_len];
                    m_write_buffer_size = m_compress_len;
                }

                // 2：判断写WriteBuffer长度是否够;
                if(m_compress_len + m_write_buffer_offset <= m_write_buffer_size)
                {
                    // 3：若长度够把压缩后的当前块数据到写Buffer当中；
                    ::memcpy(m_write_buffer + m_write_buffer_offset,
                            m_compress_buffer, m_compress_len);
                    m_write_buffer_offset += m_compress_len;

                    ReserveIndex();
                    m_written_num_blocks++;
                }
                else
                {
                    // 5：若长度不够，则将writer buffer写到xfs；
                    uint32_t error_code = 0;
                    int64_t written_len = m_file->Write(m_write_buffer,
                                                        m_write_buffer_offset,
                                                        &error_code);

                    if (error_code != ERR_FILE_OK)
                    {
                        LOG(ERROR) << "dump write buffer failed, write buffer bytes:"
                                   << m_write_buffer_offset;
                        assert(written_len == 0);
                        (void) written_len;
                        return kRetIOError;
                    }

                    // 设置header信息
                    m_sstable_header.set_raw_length(m_total_data_bytes);
                    m_sstable_header.set_compressed_len(m_total_compressed_bytes);
                    m_write_buffer_offset = 0;

                    if ((m_compress_len > m_write_buffer_size)
                        && (m_write_buffer_offset == 0))
                    {
                        // 若块压缩之后的大小大于BufferSize并且Buffer为空时，需要扩充Buffer
                        delete[] m_write_buffer;
                        m_write_buffer = new char[m_compress_len];
                        m_write_buffer_size = m_compress_len;
                    }

                    // 拷贝之前没有写入到write buffer的块
                    ::memcpy(m_write_buffer, m_compress_buffer, m_compress_len);
                    m_write_buffer_offset = m_compress_len;

                    ReserveIndex();
                    m_written_num_blocks++;
                    m_compress_len = 0;
                }

                if (ret == kRetOK)
                {
                    // reset当前写入块;
                    ret = kRetBlockOverflow;
                    m_cur_data_block->Reset();
                }
            }
        }
    }
    while (ret == kRetBlockOverflow);

    // 写BloomFilter;
    if (ret == kRetOK && m_sstable_header.options().bloomfilter_size() > 0)
    {
        m_bloomfilter->Insert(key, key_len);
        m_written_num_records++;
    }

    return ret;
}

/// @brief    保存当前block的索引;
void SSTableWriter::ReserveIndex()
{
    RetCode ret = kRetOK;
    // 写入当前块的Index信息;
    int64_t cur_offset = m_total_compressed_bytes;
    std::string last_key = m_cur_data_block->GetLastKey();
    uint32_t val_len = sizeof(uint64_t);
    uint64_t val = static_cast<uint64_t>(::htonll(cur_offset));

    do
    {
        ret = m_index_block->WriteRecord(last_key.data(),
                                         last_key.length(),
                                         reinterpret_cast<char*>(&val),
                                         val_len);

        if (ret == kRetBlockOverflow)
        {
            uint32_t index_block_size =
                m_index_description.options().block_size();

            if (index_block_size * 2 < kMaxBlockSize)
            {
                index_block_size *= 2;
                m_index_description.mutable_options()->set_block_size(
                    index_block_size);
                m_index_block->ChangeBlockSize();
            }
            else
            {
                ret = kRetFileOverflow;
            }
        }
    }
    while (ret == kRetBlockOverflow);
}

/// @brief    检测写入的record的合法性;
/// @param    key, key的指针;
/// @param    key_len, key的长度;
/// @retval   true 合法，false, 不合法;
bool SSTableWriter::CheckRecordValid(const void* key, uint16_t key_len)
{
    if (m_written_num_records == 0)
    {
        m_prev_record = std::string(static_cast<const char*>(key), key_len);
        return true;
    }

    if (CompareByteString(key, key_len, m_prev_record.data(), m_prev_record.length()) >= 0)
    {
        m_prev_record = std::string(static_cast<const char*>(key), key_len);
        return true;
    }

    return false;
}

/// @brief    向文件当中dump datablock;
/// @retval   kRetOK, 成功;
///           kRetFileOverflow, 文件已经写满,不能再写入;
///           kRetIOError, IO错误;
///           kRetCompressError, 压缩block错误
RetCode  SSTableWriter::DumpCurDataBlock()
{
    RetCode ret = kRetOK;

    if (m_written_num_blocks == kMaxNumOfRecords)
    {
        return kRetFileOverflow;
    }

    // 检测压缩内存够不够;
    uint32_t block_bytes = m_cur_data_block->GetWrittenBytes();
    uint32_t buffer_len = m_compress_buffer_size;

    if (block_bytes > buffer_len)
    {
        delete []m_compress_buffer;
        m_compress_buffer = new char[block_bytes];
        m_compress_buffer_size = block_bytes;
        buffer_len = m_compress_buffer_size;
    }

    // 压缩数据块
    ret = m_cur_data_block->CompressBlockData(m_compress_buffer, &buffer_len);

    if (ret == kRetOK)   // dump到磁盘;
    {
        m_total_data_bytes += m_cur_data_block->GetWrittenBytes();
        m_total_compressed_bytes += buffer_len;

        m_compress_len = buffer_len;
    }
    else
    {
        ret = kRetCompressError;
    }

    return ret;
}

/// @brief  向文件当中写入Bloomfilter;
/// @param  无
/// @retval kRetOk: 成功
///         kRetIOError: dump bloom filter失败
RetCode SSTableWriter::DumpBloomfilter()
{
    RetCode ret = kRetOK;

    // 没有bloomfilter块需要写入
    if (m_bloomfilter == NULL) {
        m_sstable_header.clear_bloomfilter_block();
        return kRetOK;
    }

    uint32_t error_code = 0;
    int64_t  cur_offset = m_file->Tell(&error_code);

    if (error_code != ERR_FILE_OK)
    {
        LOG(ERROR) << "dump bloomfilter tell failed, error no:"  << error_code;
        return kRetIOError;
    }

    uint64_t bloomfilter_offset = static_cast<uint64_t>(cur_offset);
    uint64_t bloomfilter_length = m_bloomfilter->MemorySize();

    int64_t written_len = 0;
    unsigned char* bitmap = m_bloomfilter->GetBitmap();
    written_len = m_file->Write(bitmap, bloomfilter_length, &error_code);

    if (error_code != ERR_FILE_OK)
    {
        assert(written_len == 0);
        LOG(ERROR) << "dump bloomfilter write failed, buffer len:"
                   << bloomfilter_length
                   << "error no:"
                   << error_code;
        return kRetIOError;
    }

    assert(written_len == static_cast<int64_t>(bloomfilter_length));
    m_sstable_header.mutable_bloomfilter_block()->set_bloomfilter_offset(
        bloomfilter_offset);
    m_sstable_header.mutable_bloomfilter_block()->set_bloomfilter_length(
        bloomfilter_length);
    m_sstable_header.mutable_bloomfilter_block()->set_bloomfilter_hash_num(
        m_bloomfilter->HashNumber());

    return ret;
}

/// @brief  向文件当中写入Index;
/// @param  无
/// @retval kRetOK:  成功
///         kRetIOError: dump index失败
RetCode SSTableWriter::DumpIndex()
{
    RetCode ret = kRetOK;

    // 检测压缩内存够不够;
    uint32_t block_bytes = m_index_block->GetWrittenBytes();
    uint32_t buffer_len = m_compress_buffer_size;

    if (block_bytes > buffer_len)
    {
        delete []m_compress_buffer;
        m_compress_buffer = new char[block_bytes];
        m_compress_buffer_size = block_bytes;
        buffer_len = m_compress_buffer_size;
    }

    // 压缩数据块
    ret = m_index_block->CompressBlockData(m_compress_buffer,
                                           &buffer_len);

    if (ret == kRetOK)   // dump到磁盘;
    {
        uint64_t total_data_bytes = m_index_block->GetUncompressedLength();
        uint32_t error_code = 0;
        int64_t  cur_offset = m_file->Tell(&error_code);

        if (error_code != ERR_FILE_OK)
        {
            LOG(ERROR) << "dump index tell failed, error no:"  << error_code;
            return kRetIOError;
        }

        int64_t written_len = m_file->Write(m_compress_buffer,
                                            buffer_len,
                                            &error_code);

        if (error_code != ERR_FILE_OK)
        {
            LOG(ERROR) << "dump index write failed, buffer len:"
                       << buffer_len
                       << "error no:"
                       << error_code;
            return kRetIOError;
        }

        assert(written_len == buffer_len);
        (void) written_len;

        m_sstable_header.set_index_offset(cur_offset);
        m_sstable_header.set_index_compressed_length(buffer_len);
        m_sstable_header.set_index_raw_length(total_data_bytes);
    }

    return ret;
}

/// @brief  向文件当中写入Schema;
/// @param  无
/// @retval kRetOK: 成功
///         kRetIOError:  dump schema失败
RetCode SSTableWriter::DumpSSTableHeader()
{
    RetCode ret = kRetOK;

    m_sstable_header.set_num_records(m_written_num_records);

    int serialized_len = m_sstable_header.ByteSize();
    uint32_t buffer_len = m_compress_buffer_size;

    if (static_cast<uint32_t>(serialized_len) > buffer_len)
    {
        delete []m_compress_buffer;
        m_compress_buffer = new char[serialized_len];
        m_compress_buffer_size = serialized_len;
        buffer_len = m_compress_buffer_size;
    }


    bool serialized_ret = m_sstable_header.SerializeToArray(m_compress_buffer, buffer_len);

    if (!serialized_ret)
    {
        LOG(ERROR) << "dump sstable header serialize failed, serialized len:"
                   << serialized_len;
        return kRetIOError;
    }

    // 写入头部信息;
    uint32_t error_code = 0;
    int64_t written_len = m_file->Write(m_compress_buffer,
                                        serialized_len,
                                        &error_code);

    if (error_code != ERR_FILE_OK)
    {
        LOG(ERROR) << "dump sstable header write header failed, serialized len:"
                   << serialized_len;
        return kRetIOError;
    }

    // 写入长度
    uint64_t serialized_written_len = ::htonll(static_cast<uint64_t>(serialized_len));
    written_len = m_file->Write(reinterpret_cast<char*>(&serialized_written_len),
                                8,
                                &error_code);

    if (error_code != ERR_FILE_OK)
    {
        LOG(ERROR) << "dump sstable header write header len failed, serialized len:"
                   << serialized_len;
        ret = kRetIOError;
    }

    return ret;
}

} // namespace sstable
