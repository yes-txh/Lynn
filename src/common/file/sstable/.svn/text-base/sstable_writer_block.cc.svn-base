#include <algorithm>
#include "common/base/byte_order.hpp"
#include "common/encoding/variant_integer.hpp"
#include "common/base/string/string_algorithm.hpp"
#include "common/compress/block_compression_codec.h"
#include "common/file/sstable/sstable_writer_block.h"

namespace sstable
{
// 构造和析构
WriterBlock::WriterBlock(const SSTableHeader* header): Block(header),
    m_written_bytes(0),
    m_prev_basekey_ptr(NULL),
    m_prev_basekey_len(0),
    m_prev_key_ptr(NULL),
    m_prev_key_len(0),
    m_prev_prefix_len(0),
    m_key_next_write_ptr(NULL),
    m_key_offset_next_write_ptr(NULL),
    m_basekey_index_next_write_ptr(NULL),
    m_val_offset_next_write_ptr(NULL),
    m_val_next_write_ptr(NULL)
{
    // 对于正在写的Block, 需要初始化块里面的分段内存区域;
    m_block_header = new BlockHeader();

    // 给block header预留kMaxBlockHeaderLen个字节
    m_written_bytes += kMaxBlockHeaderLen;

    uint32_t block_size = m_sstable_header->options().block_size();
    uint32_t cur_kv_type = m_sstable_header->options().kv_type();

    m_key_start_ptr = new char[block_size];
    m_key_next_write_ptr = m_key_start_ptr;

    if ((cur_kv_type & kKeyTypeMask) == kTypeFixedLen)
    {
        // 固定长度key的块;
        m_key_common_prefix = m_key_start_ptr;
    }
    else
    {
        // 变长度key的块;
        m_key_offset_ptr = new uint32_t[kMaxNumOfRecords];
        m_key_offset_next_write_ptr = m_key_offset_ptr;
        m_basekey_index_ptr = new uint32_t[kMaxNumOfRecords];
        m_basekey_index_next_write_ptr = m_basekey_index_ptr;
    }

    m_val_ptr = new char[block_size];
    m_val_next_write_ptr = m_val_ptr;

    if (((cur_kv_type & kValTypeMask) >> 4) == kTypeVariableLen)
    {
        // 变长val块;
        m_val_offset_ptr = new uint32_t[kMaxNumOfRecords];
        m_val_offset_next_write_ptr = m_val_offset_ptr;
    }
}

WriterBlock::~WriterBlock()
{
    delete []m_key_start_ptr;
    delete []m_val_ptr;

    uint32_t cur_kv_type = m_sstable_header->options().kv_type();

    if ((cur_kv_type & kKeyTypeMask) == kTypeVariableLen)
    {
        delete []m_key_offset_ptr;
        delete []m_basekey_index_ptr;
    }

    if (((cur_kv_type & kValTypeMask) >> 4) == kTypeVariableLen)
    {
        delete []m_val_offset_ptr;
    }
}

///  @brief   压缩Block头数据;
///  @param   buffer, 存放压缩后数据的buffer位置;
///  @param   buffer_len, 输入为buffer的长度;  输出为压缩后的长度;
///  @param   is_writter, true表示要写入到buffer;  false表示只计算长度，不写入buffer;
///  @retval  kRetOK 成功;
///           kRetCompressError 压缩失败;
RetCode WriterBlock::CompressBlockHeader(char* buffer, uint32_t* buffer_len, bool is_writer)
{
    // get block_header size;
    uint16_t serialized_len = m_block_header->ByteSize();

    // 写入block header
    bool serialized_ret = m_block_header->SerializeToArray(buffer, *buffer_len);
    if (!serialized_ret)
    {
        return kRetCompressError;
    }

    *buffer_len = serialized_len;
    return kRetOK;
}
///  @brief   压缩Block数据; 并且保存块写入的最后一个记录;
///  @param   buffer, 存放压缩后数据的buffer位置;
///  @param   buffer_len, 输入为buffer的长度;  输出为压缩后的长度;
///  @retval  kRetOK 成功;
///           kRetCompressError 压缩失败;
RetCode WriterBlock::CompressBlockData(char* buffer, uint32_t* buffer_len)
{
    assert((m_block_header != NULL) && (m_block_header->num_records() > 0));
    assert(m_sstable_header != NULL);

    RetCode  ret = kRetOK;
    uint32_t block_size = m_sstable_header->options().block_size();
    uint32_t cur_kv_type = m_sstable_header->options().kv_type();
    RecordKVType key_type = static_cast<RecordKVType>(cur_kv_type & kKeyTypeMask);
    RecordKVType val_type = static_cast<RecordKVType>((cur_kv_type & kValTypeMask) >> 4);

    // 1:将非连续的几块数据Copy到连续的内存区域;
    // WriterBlock, 可能存在已经有了Buffer的情况;
    if (m_buffer == NULL)
    {
        m_buffer = new char[block_size];
        m_buffer_len = block_size;
    }
    char* cur_write_ptr = m_buffer;

    // copy key field;
    if (key_type == kTypeFixedLen)
    {
        // 压缩定长Key区域;
        uint32_t fixed_key_len = m_sstable_header->options().fixed_key_len();

        if (m_block_header->key_max_common_prefix_len() > 0)
        {
            uint16_t left_bytes_per_key = static_cast<uint16_t>(fixed_key_len)
                                          - m_block_header->key_max_common_prefix_len();
            char* cur_key_start_ptr = m_key_common_prefix
                                      + m_block_header->key_max_common_prefix_len();
            uint32_t key_count = 0;
            assert((static_cast<uint32_t>(m_block_header->key_max_common_prefix_len())
                    + static_cast<uint32_t>(m_block_header->num_records()) * left_bytes_per_key)
                   < m_buffer_len);

            // 存储 base key
            ::memcpy(cur_write_ptr, m_key_common_prefix,
                m_block_header->key_max_common_prefix_len());
            cur_write_ptr += m_block_header->key_max_common_prefix_len();

            // 存储key
            for (; key_count < m_block_header->num_records(); key_count++)
            {
                ::memcpy(cur_write_ptr, cur_key_start_ptr, left_bytes_per_key);
                cur_key_start_ptr += static_cast<uint16_t>(fixed_key_len);
                cur_write_ptr += left_bytes_per_key;
            }
        }
        else
        {
            uint32_t fixedlen_key_field_len =
                static_cast<uint32_t>(m_block_header->num_records()) * fixed_key_len;
            assert(fixedlen_key_field_len < m_buffer_len);
            ::memcpy(cur_write_ptr, m_key_common_prefix, fixedlen_key_field_len);
            cur_write_ptr += fixedlen_key_field_len;
        }

        // 对lastkey进行赋值;
        m_last_key = std::string(m_key_start_ptr
                                 + fixed_key_len * (m_block_header->num_records() - 1),
                                 fixed_key_len);
    }
    else
    {
        // 压缩变长key;
        uint32_t key_field_len = *(m_key_offset_ptr + m_block_header->num_records() - 1);
        assert((key_field_len) < m_buffer_len);

        // copy key
        ::memcpy(cur_write_ptr, m_key_start_ptr, key_field_len);
        cur_write_ptr += key_field_len;

        // 对lastkey进行赋值;
        char* last_key_ptr = NULL;
        if (m_block_header->num_records() > 1)
        {
            last_key_ptr = m_key_start_ptr +
                             *(m_key_offset_ptr + m_block_header->num_records() - 2);
        }
        else
        {
            last_key_ptr = m_key_start_ptr;
        }

        uint16_t last_key_len = 0;
        uint32_t last_basekey_no = *(m_basekey_index_ptr + m_block_header->num_basekeys() - 1);
        char*    last_basekey_ptr = NULL;

        uint16_t prefix_len = 0;
        uint16_t skipped_bytes = 0;

        if (last_basekey_no == (m_block_header->num_records() - 1))
        {
            // 最后一个key为basekey;
            if (last_basekey_no == 0)
            {
                last_key_len = static_cast<uint16_t>(*m_key_offset_ptr);
            }
            else
            {
                last_key_len = static_cast<uint16_t>(*(m_key_offset_ptr + last_basekey_no)
                                                     - *(m_key_offset_ptr + last_basekey_no - 1));
            }

            m_last_key = std::string(last_key_ptr, last_key_len);
        }
        else
        {
            if (last_basekey_no == 0)
            {
                last_basekey_ptr = m_key_start_ptr;
            }
            else
            {
                last_basekey_ptr = m_key_start_ptr + *(m_key_offset_ptr + last_basekey_no - 1);
            }

            assert(m_block_header->num_records() > 1);
            last_key_len = static_cast<uint16_t>(*(m_key_offset_ptr
                                                   + m_block_header->num_records() - 1)
                                                 - *(m_key_offset_ptr
                                                         + m_block_header->num_records() - 2));
            // decode the prefix_len
            skipped_bytes =static_cast<uint16_t>(VariantInteger::Decode(
                            last_key_ptr, last_key_len, &prefix_len));

            m_last_key = std::string(last_basekey_ptr, prefix_len)
                         + std::string(last_key_ptr + skipped_bytes, last_key_len - skipped_bytes);
        }
    }

    uint32_t total_key_field_len = static_cast<uint32_t>
                                   (cur_write_ptr - m_buffer);
    (void) total_key_field_len;

    // copy val field;
    if (val_type == kTypeFixedLen)
    {
        uint32_t fixedlen_val_field_len = static_cast<uint32_t>(m_block_header->num_records())
                                          * m_sstable_header->options().fixed_data_len();
        assert(fixedlen_val_field_len <= (m_buffer_len - total_key_field_len));
        ::memcpy(cur_write_ptr, m_val_ptr, fixedlen_val_field_len);
        cur_write_ptr += fixedlen_val_field_len;
    }
    else
    {
        // Copy val区域;
        uint32_t val_field_len = *(m_val_offset_ptr + m_block_header->num_records() - 1);
        assert(val_field_len <= (m_buffer_len - total_key_field_len));
        ::memcpy(cur_write_ptr, m_val_ptr, val_field_len);
        cur_write_ptr += val_field_len;
    }

    uint32_t total_block_len = static_cast<uint32_t>(cur_write_ptr - m_buffer);
    assert(total_block_len <= m_buffer_len);

    char* cur_buffer_ptr = buffer;

    // 压缩数据到目标Buffer;
    intern::BlockCompressionCodec* codec =
        intern::BlockCompressionCodec::CreateCodec(m_sstable_header->options().compress_type());
    assert(codec != NULL);

    size_t  compressed_length = *buffer_len;
    //uint32_t compress_crc = 0;
    int deflate_ret = codec->Deflate(m_buffer,
                                     total_block_len,
                                     cur_buffer_ptr,
                                     &compressed_length);
                                     //compress_crc);
    delete codec;
    if (deflate_ret < 0)
    {
        return kRetCompressError;
    }

    cur_buffer_ptr += compressed_length;
    //m_block_header->set_compress_crc(compress_crc);
    m_block_header->set_raw_len(total_block_len); // 压缩之前的block数据长度

    // 压缩Offset到目标Buffer;
    uint32_t offsets_len = *buffer_len - compressed_length;
    uint32_t raw_offsets_len = 0;
    CompressOffset(cur_buffer_ptr, &offsets_len, &raw_offsets_len);
    cur_buffer_ptr += offsets_len;
    m_block_header->set_compress_offsets_len(offsets_len); // 压缩后的offsets的长度
    m_block_header->set_raw_offsets_len(raw_offsets_len); // 压缩前的offsets的长度

    // 设置头部信息，并且压缩头部到目标Buffer;
    uint32_t header_size = *buffer_len - compressed_length - offsets_len;
    ret = CompressBlockHeader(cur_buffer_ptr, &header_size);
    if (ret != kRetOK)
    {
        return ret;
    }
    cur_buffer_ptr += header_size;

    // 写入头部压缩完毕之后的长度;
    *reinterpret_cast<uint32_t*>(cur_buffer_ptr) = ByteOrder::LocalToNet(header_size);

    *buffer_len = compressed_length + offsets_len + header_size + sizeof(uint32_t);

    return ret;
}

///  @brief   压缩offset数据; 将offset数据存放为Variant Integer;
///  @param   buffer, 存放压缩后offset数据的buffer位置;
///  @param   offset, offset的指针;
///  @param   offset num, offset的个数;
void WriterBlock::CompressOffset(char* buffer, uint32_t* buffer_len, uint32_t* raw_offsets_len)
{
    uint32_t cur_kv_type = m_sstable_header->options().kv_type();
    RecordKVType key_type = static_cast<RecordKVType>(cur_kv_type & kKeyTypeMask);
    RecordKVType val_type = static_cast<RecordKVType>((cur_kv_type & kValTypeMask) >> 4);

    uint32_t offset_num = m_block_header->num_records();
    uint32_t total_write_bytes = 0;
    *raw_offsets_len = 0;
    if (key_type == kTypeVariableLen)
    {
        // compress key_offset;
        uint32_t writer_buffer_len = *buffer_len - total_write_bytes;
        EncodeOffsets<uint32_t>(buffer + total_write_bytes, &writer_buffer_len,
                               m_key_offset_ptr, offset_num);
        total_write_bytes += writer_buffer_len;
        *raw_offsets_len += offset_num * sizeof(uint32_t);

        // compress base_key offset
        uint32_t basekey_num = m_block_header->num_basekeys();
        writer_buffer_len = *buffer_len - total_write_bytes;
        EncodeOffsets<uint32_t>(buffer + total_write_bytes, &writer_buffer_len,
                               m_basekey_index_ptr, basekey_num);
        total_write_bytes += writer_buffer_len;

        *raw_offsets_len += basekey_num * sizeof(uint32_t);
    }

    if (val_type == kTypeVariableLen)
    {
        // compress value_offset;
        uint32_t writer_buffer_len = *buffer_len - total_write_bytes;
        EncodeOffsets<uint32_t>(buffer + total_write_bytes, &writer_buffer_len,
                               m_val_offset_ptr, offset_num);
        total_write_bytes += writer_buffer_len;

        *raw_offsets_len += offset_num * sizeof(uint32_t);
    }

    *buffer_len = total_write_bytes;
}

///  @brief   向Block当中写入一条数据;
///  @param   key, 写入记录的key
///  @param   key_len, 写入记录的key len
///  @param   data, 写入记录的data
///  @param   data_len, 写入记录的data len
///  @retval  kRetOK: 写入成功
///           kRetBlockOverflow: Block已满
RetCode WriterBlock::WriteRecord(const void* key,
                                 uint16_t key_len,
                                 const void* val,
                                 uint32_t val_len)
{
    assert((key != NULL) && (key_len != 0));
    assert((m_block_header != NULL) && (m_key_start_ptr != NULL) && (m_sstable_header != NULL));
    assert((m_key_next_write_ptr != NULL) && (m_val_next_write_ptr != NULL));

    RetCode ret = kRetOK;

    if ((m_sstable_header->options().kv_type() & kKeyTypeMask) == kTypeFixedLen)
    {
        ret = WriteFixedKeyLenRecord(key, key_len, val, val_len);
    }
    else
    {
        ret = WriteVarKeyLenRecord(key, key_len, val, val_len);
    }

    return ret;
}

///  @brief   固定长度key的记录的写入;
///  @param   key, 写入记录的key
///  @param   key_len, 写入记录的key len
///  @param   data, 写入记录的data
///  @param   data_len, 写入记录的data len
///  @retval  kRetOK: 写入成功
///           kRetBlockOverflow: Block已满,没写成功;
RetCode WriterBlock::WriteFixedKeyLenRecord(const void* key,
        uint16_t key_len,
        const void* val,
        uint32_t val_len)
{
    RetCode  ret = kRetOK;
    assert(key_len == m_sstable_header->options().fixed_key_len());
    RecordKVType val_type =
        static_cast<RecordKVType>(
            (m_sstable_header->options().kv_type() & kValTypeMask) >> 4);

    uint32_t  cur_written_len = 0;

    if (val_type == kTypeVariableLen)
    {
        cur_written_len = key_len + val_len + sizeof(uint32_t);
    }
    else
    {
        cur_written_len = key_len + val_len;
    }

    // 1: 判断已经写入的长度;
    if (((m_written_bytes + cur_written_len) > m_sstable_header->options().block_size())
        || (m_block_header->num_records() >= kMaxNumOfRecords))
    {
        ret = kRetBlockOverflow;
    }
    else
    {
        // 1.1: 写入Key数据;
        ::memcpy(m_key_next_write_ptr, key, key_len);
        m_key_next_write_ptr += key_len;

        // 1.2: 求Key的最大公共前缀串;
        if (m_block_header->num_records() == 0)
        {
            // block中的第一条记录;
            m_block_header->set_key_max_common_prefix_len(key_len);
        }
        else
        {
            m_block_header->set_key_max_common_prefix_len(
                GetCommonPrefixLength(m_key_common_prefix,
                                      m_block_header->key_max_common_prefix_len(),
                                      key,
                                      key_len));
        }

        ::memcpy(m_val_next_write_ptr, val, val_len);
        m_val_next_write_ptr += val_len;

        if (val_type == kTypeVariableLen)
        {
            // 每个Record的Offset处写入下一条记录的起始位置;
            *m_val_offset_next_write_ptr =
                static_cast<uint32_t>(m_val_next_write_ptr - m_val_ptr);
            m_val_offset_next_write_ptr++;
        }
        else
        {
            assert(val_len == m_sstable_header->options().fixed_data_len());
        }

        m_written_bytes += cur_written_len;
        m_block_header->set_num_records(m_block_header->num_records() + 1);
    }

    return ret;
}

///  @brief   固定长度key的记录的写入;
///  @param   key, 写入记录的key
///  @param   key_len, 写入记录的key len
///  @param   data, 写入记录的data
///  @param   data_len, 写入记录的data len
///  @retval  kRetOK: 写入成功
///           kRetBlockOverflow: Block已满,没写成功;
RetCode WriterBlock::WriteVarKeyLenRecord(const void* key,
        uint16_t key_len,
        const void* val,
        uint32_t val_len)
{
    RetCode  ret = kRetOK;

    uint32_t      cur_write_len  = 0;
    uint16_t      cur_prefix_len = 0;
    RecordKVType  val_type =
        static_cast<RecordKVType>(
            (m_sstable_header->options().kv_type() & kValTypeMask) >> 4);

    // 判断是否需要自立门户，独立成一个basekey;
    bool should_be_base = ShouldBeBase(key, key_len, &cur_prefix_len);

    if (should_be_base)
    {
        if (val_type == kTypeVariableLen)
        {
            cur_write_len = key_len + val_len + 2 * sizeof(uint32_t) + sizeof(uint32_t);
        }
        else
        {
            cur_write_len = key_len + val_len + sizeof(uint32_t) + sizeof(uint32_t);
        }

        if (((m_written_bytes + cur_write_len) > m_sstable_header->options().block_size())
            || (m_block_header->num_records() >= kMaxNumOfRecords))
        {
            return kRetBlockOverflow;
        }

        m_prev_key_ptr = m_key_next_write_ptr;
        m_prev_key_len = key_len;
        m_prev_basekey_ptr = m_key_next_write_ptr;
        m_prev_basekey_len = key_len;
        m_prev_prefix_len = 0;

        // 写入key;
        ::memcpy(m_key_next_write_ptr, key, key_len);
        m_key_next_write_ptr += key_len;

        // 写入basekey的recordno;
        *m_basekey_index_next_write_ptr = m_block_header->num_records();
        m_basekey_index_next_write_ptr++;
        m_written_bytes += key_len + sizeof(uint32_t);
        m_block_header->set_num_basekeys(m_block_header->num_basekeys() + 1);
    }
    else
    {
        // check buffer size;
        uint16_t encoding_size =
            static_cast<uint16_t>(VariantInteger::EncodedSize<uint16_t>(cur_prefix_len));

        if (val_type == kTypeVariableLen)
        {
            cur_write_len = encoding_size + key_len
                            - cur_prefix_len + val_len + 2 * sizeof(uint32_t);
        }
        else
        {
            cur_write_len = encoding_size + key_len
                            - cur_prefix_len + val_len + sizeof(uint32_t);
        }

        if (((m_written_bytes + cur_write_len) > m_sstable_header->options().block_size())
            || (m_block_header->num_records() >= kMaxNumOfRecords))
        {
            return kRetBlockOverflow;
        }

        // 直接根据前一个basekey进行prefix压缩;
        VariantInteger::UncheckedEncode<uint16_t>(cur_prefix_len, m_key_next_write_ptr);
        m_key_next_write_ptr += encoding_size;
        ::memcpy(m_key_next_write_ptr, static_cast<const char*>(key) + cur_prefix_len, key_len - cur_prefix_len);
        m_key_next_write_ptr += key_len - cur_prefix_len;

        m_written_bytes += encoding_size + key_len - cur_prefix_len;
        m_prev_prefix_len = cur_prefix_len;
    }

    // 写入key,offset
    *m_key_offset_next_write_ptr =
        static_cast<uint32_t>(m_key_next_write_ptr - m_key_start_ptr);
    m_key_offset_next_write_ptr++;
    m_written_bytes += sizeof(uint32_t);

    // 前面已经进行过缓存区大小的判断,直接写入值;
    ::memcpy(m_val_next_write_ptr, val, val_len);
    m_val_next_write_ptr += val_len;

    if (val_type == kTypeVariableLen)
    {
        // 每个Record的Offset处写入下一条记录的起始位置;
        *m_val_offset_next_write_ptr =
            static_cast<uint32_t>(m_val_next_write_ptr - m_val_ptr);
        m_val_offset_next_write_ptr++;
        m_written_bytes += val_len + sizeof(uint32_t);
    }
    else
    {
        assert(val_len == m_sstable_header->options().fixed_data_len());
        m_written_bytes += val_len;
    }

    m_block_header->set_num_records(m_block_header->num_records() + 1);

    return ret;
}


///  @breif  判断一个待写入的key是否应该独立成basekey;
///  @param  key 需要写入的key;
///  @param  key_len key的长度;
///  @param  prefix_len, 若不需要为base时,返回和当前base的prefix_len
///  @retval true, 需要为base;
///          fasle,不需要为base;
bool WriterBlock::ShouldBeBase(const void* key,
                               uint16_t key_len,
                               uint16_t* prefix_len) const
{
    bool should_be_base = false;
    uint16_t base_prefix_len = 0;
    uint16_t common_prefix_len = 0;
    uint32_t threshold_len = m_sstable_header->options().threshold_len();

    if (m_block_header->num_records() == 0)
    {
        return true;
    }

    if (m_prev_key_ptr == m_prev_basekey_ptr)   /// 前一条key为basekey;
    {
        // 计算本key相对与前一条记录的basekey的前缀压缩串长度;
        base_prefix_len = GetCommonPrefixLength(key, key_len,
                                                m_prev_basekey_ptr, m_prev_basekey_len);

        if (base_prefix_len < threshold_len)
        {
            // 前缀串太短,需要独立成basekey;
            should_be_base = true;
        }
        else
        {
            *prefix_len = base_prefix_len;
        }
    }
    else
    {
        ///  计算本key相对于前一条key的前缀压缩串长度;
        assert(m_prev_prefix_len > threshold_len);
        common_prefix_len = GetCommonPrefixLength(key, key_len,
                            m_prev_basekey_ptr, m_prev_prefix_len);

        if (common_prefix_len < m_prev_prefix_len)
        {
            if (common_prefix_len <= threshold_len)
            {
                /// 前缀串太短,需要独立成basekey;
                should_be_base = true;
            }
            else
            {
                *prefix_len = common_prefix_len;
            }
        }
        else     /// common prefix长度和m_prev_prefix_len相等;
        {
            uint16_t prev_prefix_len = GetCommonPrefixLength(static_cast<const char*>(key) + common_prefix_len,
                                       key_len - common_prefix_len, m_prev_key_ptr, m_prev_key_len);


            if (prev_prefix_len >  m_sstable_header->options().threshold_diff())
            {
                should_be_base = true;
            }
            else
            {
                *prefix_len = common_prefix_len;
            }
        }
    }

    return should_be_base;
}

} // namespace
