#include <algorithm>
#include "common/base/byte_order.hpp"
#include "common/base/string/string_algorithm.hpp"
#include "common/system/memory/unaligned.hpp"
#include "common/compress/block_compression_codec.h"
#include "common/file/sstable/sstable_reader_block.h"

namespace sstable
{
// 构造和析构
ReaderBlock::ReaderBlock(const SSTableHeader* header):
    Block(header),
    m_ref(0),
    m_memory_from_cache(false)
{}

ReaderBlock::~ReaderBlock() {}


/// @brief   获取压缩的key; 用于iterator比较大小时使用; 避免一次内存Copy;
/// @param   record_no, 记录的下标索引号;
/// @param   base_index, 标识base串record No的数组下标;
/// @param   prefix_combine_key, key的指针;
/// @retval
void ReaderBlock::GetPrefixCompressedKey(uint32_t record_no, uint32_t base_index,
        PrefixCompressedKey* prefix_compressed_key) const
{
    assert(prefix_compressed_key != NULL);
    assert((m_block_header != NULL) && (record_no < m_block_header->num_records()));
    assert(m_key_start_ptr != NULL);
    assert(m_sstable_header != NULL);

    if ((m_sstable_header->options().kv_type() & kKeyTypeMask) == kTypeFixedLen)
    {
        assert(m_key_common_prefix != NULL);
        uint16_t left_key_len =
            static_cast<uint16_t>(m_sstable_header->options().fixed_key_len())
            - m_block_header->key_max_common_prefix_len();
        prefix_compressed_key->remainder_key = m_key_start_ptr + left_key_len * record_no;
        prefix_compressed_key->remainder_key_len = left_key_len;
        prefix_compressed_key->prefix_key = m_key_common_prefix;
        prefix_compressed_key->prefix_key_len = m_block_header->key_max_common_prefix_len();
        return;
    }

    assert(base_index < m_block_header->num_basekeys());
    assert((m_key_offset_ptr != NULL) && (m_basekey_index_ptr != NULL));
    char* cur_key = NULL;
    char* cur_base_key = NULL;
    uint16_t cur_key_len = 0;
    uint16_t cur_base_key_len = 0;
    uint32_t base_key_record_no = 0;
    uint16_t prefix_len = 0;
    uint16_t skipped_bytes = 0;

    /// 1: 求key的整长度;
    if (record_no == 0)
    {
        prefix_compressed_key->remainder_key = m_key_start_ptr;
        prefix_compressed_key->remainder_key_len = *m_key_offset_ptr;
        prefix_compressed_key->prefix_key = NULL;
        prefix_compressed_key->prefix_key_len = 0;
    }
    else
    {
        cur_key = m_key_start_ptr + *(m_key_offset_ptr + record_no - 1);
        cur_key_len = static_cast<uint16_t>(*(m_key_offset_ptr + record_no)
                                            - *(m_key_offset_ptr + record_no - 1));
        base_key_record_no = *(m_basekey_index_ptr + base_index);

        if (base_key_record_no == record_no)
        {
            prefix_compressed_key->prefix_key = NULL;
            prefix_compressed_key->prefix_key_len = 0;
            prefix_compressed_key->remainder_key = cur_key;
            prefix_compressed_key->remainder_key_len = cur_key_len;
            return;
        }

        if (base_key_record_no == 0)
        {
            cur_base_key = m_key_start_ptr;
            cur_base_key_len = static_cast<uint16_t>(*m_key_offset_ptr);
        }
        else
        {
            cur_base_key = m_key_start_ptr +
                *(m_key_offset_ptr + base_key_record_no - 1);
            cur_base_key_len = static_cast<uint16_t>(*(m_key_offset_ptr +
                base_key_record_no) - *(m_key_offset_ptr + base_key_record_no - 1));
        }
        skipped_bytes = static_cast<uint16_t>(VariantInteger::Decode(cur_key,
            cur_key_len, &prefix_len));

        prefix_compressed_key->prefix_key = cur_base_key;
        prefix_compressed_key->prefix_key_len = prefix_len;
        prefix_compressed_key->remainder_key = cur_key + skipped_bytes;
        prefix_compressed_key->remainder_key_len = cur_key_len - skipped_bytes;
    }
}


/// @brief   依据Record No获取key, 将key的值Copy到给定的Buffer当中;
/// @param   record_no, 记录号
/// @param   base_index, 标识base串record No的数组下标;
/// @param   key,       指向key的Buffer指针;
/// @param   key_len,   传入时为存放key的Buffer长度, 传出时为Key的实际长度;
/// @retval  kRetOK, 成功;
///          kRetBufferOverflow, 内存不够;
RetCode ReaderBlock::GetKeyByRecordNo(uint32_t record_no, uint32_t base_index,
                                      char* key, uint16_t* key_len) const
{
    assert(key_len != NULL);
    RetCode ret = kRetOK;
    /// 1: 求key的整长度;
    PrefixCompressedKey prefix_compressed_key;
    GetPrefixCompressedKey(record_no, base_index, &prefix_compressed_key);

    if ((prefix_compressed_key.remainder_key_len
         + prefix_compressed_key.prefix_key_len) > (*key_len))   // 内存溢出;
    {
        (*key_len) = prefix_compressed_key.remainder_key_len
                     + prefix_compressed_key.prefix_key_len;
        return kRetBlockOverflow;
    }

    if (prefix_compressed_key.prefix_key_len > 0)
    {
        ::memcpy(key, prefix_compressed_key.prefix_key,
                 prefix_compressed_key.prefix_key_len);
        ::memcpy(key + prefix_compressed_key.prefix_key_len,
                 prefix_compressed_key.remainder_key,
                 prefix_compressed_key.remainder_key_len);
    }
    else
    {
        ::memcpy(key, prefix_compressed_key.remainder_key,
                 prefix_compressed_key.remainder_key_len);
    }

    (*key_len) = prefix_compressed_key.prefix_key_len
                 + prefix_compressed_key.remainder_key_len;


    return ret;
}

void ReaderBlock::GetKeyByRecordNo(uint32_t record_no, uint32_t base_index,
                                   std::string* key) const
{
    assert(key != NULL);
    PrefixCompressedKey prefix_compressed_key;
    GetPrefixCompressedKey(record_no, base_index, &prefix_compressed_key);

    if (prefix_compressed_key.prefix_key_len > 0)
    {
        *key = std::string(prefix_compressed_key.prefix_key,
                           prefix_compressed_key.prefix_key_len);
        *key += std::string(prefix_compressed_key.remainder_key,
                            prefix_compressed_key.remainder_key_len);
    }
    else
    {
        *key = std::string(prefix_compressed_key.remainder_key,
                           prefix_compressed_key.remainder_key_len);
    }
}

/// @brief   获取val的指针;
/// @param   record_no, 记录号
/// @param   val,       输出参数, 存放值的Buffer指针的位置，
/// @param   val_len，  输出参数, 传入时为存放key的Buffer长度, 传出时为Key的实际长度;
void ReaderBlock::GetValPtrByRecordNo(uint32_t record_no,
                                      char** val, uint32_t* val_len) const
{
    assert(val_len != NULL);
    assert((m_block_header != NULL) && (record_no < m_block_header->num_records()));
    assert(m_sstable_header != NULL);
    RecordKVType  val_type = static_cast<RecordKVType>(
        m_sstable_header->options().kv_type());
    val_type = static_cast<RecordKVType>((val_type & kValTypeMask) >> 4);
    /// 1: 计算值的长度和Offset;
    uint32_t   cur_val_offset = 0;
    uint32_t   cur_val_len = 0;

    if (val_type == kTypeVariableLen)
    {
        if (record_no == 0)
        {
            cur_val_len = *m_val_offset_ptr;
        }
        else
        {
            cur_val_offset = *(m_val_offset_ptr + record_no - 1);
            cur_val_len = *(m_val_offset_ptr + record_no) - cur_val_offset;
        }
    }
    else
    {
        cur_val_len = m_sstable_header->options().fixed_data_len();
        cur_val_offset = record_no * cur_val_len;
    }

    if (val != NULL)  // 传入的指针为NULL时, 相当于只取长度;
    {
        *val = m_val_ptr + cur_val_offset;
    }

    (*val_len) = cur_val_len;
}

void ReaderBlock::GetValPtrByRecordNo(uint32_t record_no, StringPiece* val) const
{
    assert(val != NULL);
    assert((m_block_header != NULL) && (record_no < m_block_header->num_records()));
    assert(m_sstable_header != NULL);
    RecordKVType  val_type = static_cast<RecordKVType>(
        m_sstable_header->options().kv_type());
    val_type = static_cast<RecordKVType>((val_type & kValTypeMask) >> 4);

    /// 1: 计算值的长度和Offset;
    uint32_t   cur_val_offset = 0;
    uint32_t   cur_val_len = 0;

    if (val_type == kTypeVariableLen)
    {
        if (record_no == 0)
        {
            cur_val_len = *m_val_offset_ptr;
        }
        else
        {
            cur_val_offset = *(m_val_offset_ptr + record_no - 1);
            cur_val_len = *(m_val_offset_ptr + record_no) - cur_val_offset;
        }
    }
    else
    {
        cur_val_len = m_sstable_header->options().fixed_data_len();
        cur_val_offset = record_no * cur_val_len;
    }

    val->set(m_val_ptr + cur_val_offset, cur_val_len);
}

/// @brief   获取val;   将值Copy到给定的Buffer当中;
/// @param   record_no, 记录号
/// @param   val,       存放值的Buffer，
/// @param   val_len，  传入时为存放key的Buffer长度, 传出时为Key的实际长度;
/// @retval  kRetOK, 成功;
///          kRetBufferOverflow, 内存不够;
RetCode ReaderBlock::GetValByRecordNo(uint32_t record_no,
                                      void* val, uint32_t* val_len) const
{
    assert(val_len != NULL);
    assert((m_block_header != NULL) && (record_no < m_block_header->num_records()));
    assert(m_sstable_header != NULL);

    RetCode ret = kRetOK;
    RecordKVType  val_type = static_cast<RecordKVType>(
        m_sstable_header->options().kv_type());
    val_type = static_cast<RecordKVType>((val_type & kValTypeMask) >> 4);

    /// 1: 计算值的长度和Offset;
    uint32_t   cur_val_offset = 0;
    uint32_t   cur_val_len = 0;

    if (val_type == kTypeVariableLen)
    {
        if (record_no == 0)
        {
            cur_val_len = *m_val_offset_ptr;
        }
        else
        {
            cur_val_offset = *(m_val_offset_ptr + record_no - 1);
            cur_val_len = *(m_val_offset_ptr + record_no) - cur_val_offset;
        }

        if ((*val_len) < cur_val_len)
        {
            (*val_len) = cur_val_len;
            return kRetBufferOverflow;
        }
    }
    else
    {
        cur_val_len = m_sstable_header->options().fixed_data_len();
        if ((*val_len) < cur_val_len)
        {
            (*val_len) = cur_val_len;
            return kRetBufferOverflow;
        }

        cur_val_offset = record_no * cur_val_len;
    }

    // 2: Copy内存;
    ::memcpy(val, m_val_ptr + cur_val_offset, cur_val_len);
    (*val_len) = cur_val_len;
    return ret;
}

void ReaderBlock::GetValByRecordNo(uint32_t record_no, std::string* val) const
{
    assert(val != NULL);
    assert((m_block_header != NULL) && (record_no < m_block_header->num_records()));
    assert(m_sstable_header != NULL);

    RecordKVType  val_type = static_cast<RecordKVType>(
        m_sstable_header->options().kv_type());
    val_type = static_cast<RecordKVType>((val_type & kValTypeMask) >> 4);
    /// 1: 计算值的长度和Offset;
    uint32_t   cur_val_offset = 0;
    uint32_t   cur_val_len     = 0;

    if (val_type == kTypeVariableLen)
    {
        if (record_no == 0)
        {
            cur_val_len = *m_val_offset_ptr;
        }
        else
        {
            cur_val_offset = *(m_val_offset_ptr + record_no - 1);
            cur_val_len = *(m_val_offset_ptr + record_no) - cur_val_offset;
        }
    }
    else
    {
        cur_val_len = m_sstable_header->options().fixed_data_len();
        cur_val_offset = record_no * cur_val_len;
    }

    // 2: Copy内存;
    *val = std::string(m_val_ptr + cur_val_offset, cur_val_len);
}

/// @brief   获取key,val pair;
/// @param   record_no, 记录号
/// @param   base_index, 标识base串record No的数组下标;
/// @param   key, key的指针;
/// @param   key_len, key的长度;
/// @param   val, 值的指针;
/// @param   val_len; 值的长度;
/// @retval  kRetOK, 成功;
///          kRetBufferOverflow, 内存不够;
RetCode ReaderBlock::GetKVByRecordNo(uint32_t record_no, uint32_t base_index,
                                     char* key, uint16_t* key_len,
                                     char* val, uint32_t* val_len) const
{
    assert((key_len != NULL) && (val_len != NULL));
    assert((m_block_header != NULL) && (record_no < m_block_header->num_records()));
    assert(m_sstable_header != NULL);

    RetCode   ret = kRetOK;
    // 1: 取key的前缀串和压缩后的剩余串;
    PrefixCompressedKey prefix_compressed_key;
    GetPrefixCompressedKey(record_no, base_index, &prefix_compressed_key);

    RecordKVType  val_type = static_cast<RecordKVType>(
        m_sstable_header->options().kv_type());
    val_type = static_cast<RecordKVType>((val_type & kValTypeMask) >> 4);

    /// 2: 计算值的长度和Offset;
    uint32_t  cur_val_offset = 0;
    uint32_t  cur_val_len = 0;

    if (val_type == kTypeVariableLen)
    {
        if (record_no == 0)
        {
            cur_val_len = *m_val_offset_ptr;
        }
        else
        {
            cur_val_offset = *(m_val_offset_ptr + record_no - 1);
            cur_val_len = *(m_val_offset_ptr + record_no) - cur_val_offset;
        }
    }
    else
    {
        cur_val_len = m_sstable_header->options().fixed_data_len();
        cur_val_offset = record_no * cur_val_len;
    }

    if (((prefix_compressed_key.remainder_key_len
          + prefix_compressed_key.prefix_key_len) > (*key_len))
        || ((*val_len) < cur_val_len))   // 内存溢出;
    {
        (*key_len) = prefix_compressed_key.remainder_key_len
                     + prefix_compressed_key.prefix_key_len;
        (*val_len) = cur_val_len;
        return kRetBlockOverflow;
    }

    if (prefix_compressed_key.prefix_key_len > 0)
    {
        ::memcpy(key,
                 prefix_compressed_key.prefix_key,
                 prefix_compressed_key.prefix_key_len);
        ::memcpy(key + prefix_compressed_key.prefix_key_len,
                 prefix_compressed_key.remainder_key,
                 prefix_compressed_key.remainder_key_len);
    }
    else
    {
        ::memcpy(key,
                 prefix_compressed_key.remainder_key,
                 prefix_compressed_key.remainder_key_len);
    }

    (*key_len) = prefix_compressed_key.prefix_key_len
                 + prefix_compressed_key.remainder_key_len;

    ::memcpy(val, m_val_ptr + cur_val_offset, cur_val_len);
    (*val_len) = cur_val_len;

    return ret;
}

/// @brief   获取下一个record的base_index;
/// @param   cur_record_no, 当前的record_no;
/// @param   cur_base_index, 当前的base_index;
/// @retval  record_no+1的记录的base_index;
uint32_t ReaderBlock::GetNextBaseIndex(uint32_t cur_record_no,
        uint32_t cur_base_index) const
{
    assert((m_block_header != NULL)
           && (cur_record_no < (m_block_header->num_records())));

    RecordKVType cur_kv_type = static_cast<RecordKVType>(
        m_sstable_header->options().kv_type());
    RecordKVType key_type =  static_cast<RecordKVType>(cur_kv_type & kKeyTypeMask);
    if (key_type == kTypeFixedLen)
    {
        return 0;
    }

    assert(m_basekey_index_ptr != NULL);

    uint32_t ret = cur_base_index;

    if (cur_base_index < (m_block_header->num_basekeys() - 1))
    {
        uint32_t next_basekey_record_no = *(m_basekey_index_ptr + cur_base_index + 1);

        if (next_basekey_record_no == (cur_record_no + 1))
        {
            ret = cur_base_index + 1;
        }
    }

    return ret;
}

/// @brief   解析内存当中的非压缩Block;
/// @param   buffer, 输入的buffer;
/// @param   buffer_len, 输入的buffer len;
/// @retval
void ReaderBlock::ParseBlockData(const char* buffer, uint32_t buffer_len)
{
    assert((buffer != NULL));
    assert(m_sstable_header != NULL);
    RetCode ret = kRetOK;

    RecordKVType cur_kv_type = static_cast<RecordKVType>(
        m_sstable_header->options().kv_type());
    RecordKVType key_type =  static_cast<RecordKVType>(cur_kv_type & kKeyTypeMask);
    RecordKVType val_type = static_cast<RecordKVType>((cur_kv_type & kValTypeMask) >> 4);

    /// 注意,读Block的Buffer,
    /// assert, 避免此处引起内存泄漏;
    assert(((m_buffer == NULL) && (m_buffer_len == 0))
           || ((m_buffer == buffer) && (m_buffer_len == buffer_len)));

    m_buffer = const_cast<char*>(buffer);
    m_buffer_len = buffer_len;

    // 获取压缩后的头部长度
    char* header_size_ptr = const_cast<char*>(buffer) + buffer_len - sizeof(uint32_t);
    uint32_t compressed_header_size = *reinterpret_cast<uint32_t*>(header_size_ptr);

    // 解析block header
    char* header_ptr = header_size_ptr - compressed_header_size;
    ret = UnCompressBlockHeader(header_ptr, compressed_header_size);
    assert(ret == kRetOK);

    assert((buffer != NULL) && (buffer_len > compressed_header_size));

    uint32_t key_field_len = 0;
    uint32_t total_parsed_len = m_block_header->raw_len();

    // 给m_key_offset_ptr 以及 m_basekey_index_ptr赋值
    if (key_type == kTypeVariableLen)
    {
        m_key_offset_ptr = reinterpret_cast<uint32_t*>(m_buffer + total_parsed_len);
        total_parsed_len += m_block_header->num_records() * sizeof(uint32_t);
        m_basekey_index_ptr = reinterpret_cast<uint32_t*>(m_buffer + total_parsed_len);
        total_parsed_len += m_block_header->num_basekeys() * sizeof(uint32_t);
    }

    if (val_type == kTypeVariableLen)
    {
        m_val_offset_ptr = reinterpret_cast<uint32_t*>(m_buffer + total_parsed_len);
        total_parsed_len += m_block_header->num_records() * sizeof(uint32_t);
    }

    // 给 m_key_start_ptr m_val_ptr赋值
    if (key_type == kTypeFixedLen)
    {
        m_key_common_prefix = m_buffer;
        m_key_start_ptr = m_key_common_prefix + m_block_header->key_max_common_prefix_len();
        key_field_len = (m_sstable_header->options().fixed_key_len()
                         - m_block_header->key_max_common_prefix_len())
                        * m_block_header->num_records()
                        + m_block_header->key_max_common_prefix_len();
    }
    else
    {
        m_key_start_ptr = m_buffer;
        key_field_len = *(m_key_offset_ptr + m_block_header->num_records() - 1);
    }

    uint32_t val_field_len = 0;
    m_val_ptr = m_buffer + key_field_len;
    if (val_type == kTypeFixedLen)
    {
        val_field_len += m_block_header->num_records()
                            * m_sstable_header->options().fixed_data_len();
    }
    else
    {
        val_field_len += *(m_val_offset_ptr + m_block_header->num_records() - 1);
    }

    assert(key_field_len + val_field_len == m_block_header->raw_len());
    total_parsed_len += sizeof(uint32_t) + compressed_header_size;
    assert(total_parsed_len == m_buffer_len);
}

/// @brief    依照某个record的key在块中查找到块号;
/// @param    key, 查询key的Buffer
/// @param    key_len, 查询key len
/// @param    record_no, 需要查找的record No;
/// @param    base_index, 标识base串record No 的数组下标;
/// @param    flag: kAccurateFind = 0, /// <精确查找，
///                 kLowBoundFind = 1  /// <lowbound查找,
/// @param    is_accurated_hit, 在low_bound查找时，返回是否精确命中;
/// @retval   kRetOK:            成功
///           kRetRecordNotFind: Not find
RetCode ReaderBlock::FindRecordNoByKey(const void* key, uint16_t key_len,
                                       uint32_t* record_no, uint32_t* base_index,
                                       uint32_t flag, bool* is_accurated_hit) const
{
    assert((m_buffer != NULL) && (m_buffer_len != 0));
    assert((key != NULL) && (key_len != 0));
    assert((flag == ReaderBlock::kLowBoundFind) || (flag == ReaderBlock::kAccurateFind));
    assert((m_block_header != NULL)
           && ((m_key_common_prefix != NULL) || (m_key_start_ptr != NULL))
           && (m_sstable_header != NULL));
    assert((record_no != NULL) && (base_index != NULL));

    RetCode ret = kRetOK;

    if ((m_sstable_header->options().kv_type() & kKeyTypeMask) == kTypeFixedLen)
    {
        ret = FixedKeyLenFind(key, key_len, record_no, base_index, flag, is_accurated_hit);
    }
    else
    {
        ret = VarKeyLenFind(key, key_len, record_no, base_index, flag, is_accurated_hit);
    }

    return ret;
}


/// @brief   解析block_header;
/// @param   buffer, 输入的buffer
/// @param   buffer_len, 输入的buffer len
/// @retval  kRetOK, 成功;
///          kRetUnCompressError, 解压失败;
RetCode ReaderBlock::UnCompressBlockHeader(const char* buffer, uint32_t buffer_len)
{
    if (m_block_header == NULL)
    {
        m_block_header = new BlockHeader();
        bool parse_ret = m_block_header->ParseFromArray(buffer, buffer_len);
        if (!parse_ret)
        {
            delete m_block_header;
            m_block_header = NULL;
            return kRetUnCompressError;
        }
    }

    return kRetOK;
}

/// @brief   将文件中读取出来的数据解析到Block的Buffer当中;
/// @param   buffer, 输入的buffer
/// @param   buffer_len, 输入的buffer len
/// @retval  kRetOK, 成功;
///          kRetUnCompressError, 解压失败;
RetCode ReaderBlock::UnCompressBlockData(const void* buffer, uint32_t buffer_len)
{
    /// 解压只会在读里面使用, 读的时候确保是用的新Block;
    assert((m_buffer == NULL) && (m_buffer_len == 0) && (m_sstable_header != NULL));
    RetCode ret = kRetOK;

    // 获取压缩后的头部长度
    const char* header_size_ptr = static_cast<const char*>(buffer) + buffer_len - sizeof(uint32_t);
    uint32_t compressed_header_size = ByteOrder::NetToLocal(GetUnaligned<uint32_t>(header_size_ptr));

    // 解析block header
    const char* header_ptr = header_size_ptr - compressed_header_size;
    ret = UnCompressBlockHeader(header_ptr, compressed_header_size);
    if (ret != kRetOK)
    {
        return ret;
    }

    m_buffer_len = m_block_header->raw_len() + m_block_header->raw_offsets_len()
                    + compressed_header_size + sizeof(uint32_t);
    m_buffer = new char[m_buffer_len];

    // 解析数据key, val
    intern::BlockCompressionCodec* codec =
        intern::BlockCompressionCodec::CreateCodec(
            m_sstable_header->options().compress_type());
    assert(codec != NULL);

    const char* cur_buffer_ptr = static_cast<const char*>(buffer);
    char* cur_writer_buffer_ptr = m_buffer;
    size_t uncompressed_length = m_buffer_len;
    uint32_t compressed_length = buffer_len - sizeof(uint32_t) - m_block_header->compress_offsets_len()
                                - compressed_header_size;

    int deflate_ret = codec->Inflate(cur_buffer_ptr,
                                     compressed_length,
                                     cur_writer_buffer_ptr,
                                     &uncompressed_length);
                                     //(uint32_t)(m_block_header->compress_crc()));
    delete codec;
    if (deflate_ret < 0)
    {
        delete [] m_buffer;
        m_buffer = NULL;
        m_buffer_len = 0;
        ret = kRetUnCompressError;
    }

    // 解析offsets
    cur_buffer_ptr += compressed_length;
    cur_writer_buffer_ptr += uncompressed_length;
    uint32_t offsets_len = m_buffer_len - uncompressed_length;
    uint32_t compressed_offsets_len = m_block_header->compress_offsets_len();
    UnCompressOffset(cur_writer_buffer_ptr, cur_buffer_ptr,
                     &offsets_len, &compressed_offsets_len);

    cur_buffer_ptr += compressed_offsets_len;
    cur_writer_buffer_ptr += offsets_len;
    // copy header
    ::memcpy(cur_writer_buffer_ptr, cur_buffer_ptr, compressed_header_size);

    cur_writer_buffer_ptr += compressed_header_size;
    // copy header length
    *reinterpret_cast<uint32_t*>(cur_writer_buffer_ptr) = compressed_header_size;

    /// 数据正常解压完成; 需要对块进行Parse;
    ParseBlockData(m_buffer, m_buffer_len);

    return ret;
}


/// @brief   将文件中读取出来的数据解析到Block的Buffer当中;
/// @param   w_buffer, 输出的buffer
/// @param   r_buffer, 输入的buffer
/// @param   w_len,    输出的buffer len
/// @param   r_len,    输入的buffer len
/// @retval  kRetOK, 成功;
///          kRetUnCompressError, 解压失败;
void ReaderBlock::UnCompressOffset(char* w_buffer, const char* r_buffer,
                                      uint32_t* w_len, uint32_t* r_len)
{
    assert(m_block_header != NULL);
    assert(m_sstable_header != NULL);

    RecordKVType cur_kv_type = static_cast<RecordKVType>(
        m_sstable_header->options().kv_type());
    RecordKVType key_type =  static_cast<RecordKVType>(cur_kv_type & kKeyTypeMask);
    RecordKVType val_type = static_cast<RecordKVType>((cur_kv_type & kValTypeMask) >> 4);

    uint32_t record_num = m_block_header->num_records();
    uint32_t read_len_bytes = 0;

    char* cur_writer_ptr = w_buffer;
    const char* cur_buffer_ptr = r_buffer;

    // 如果是变长key，需要还原key_offset, basekey_index
    if (key_type == kTypeVariableLen)
    {
        // 还原key_offset
        uint32_t* key_offset_ptr = reinterpret_cast<uint32_t*>(cur_writer_ptr);

        uint32_t reader_buffer_len = *r_len - read_len_bytes;
        DecodeOffsets<uint32_t>(key_offset_ptr, record_num, cur_buffer_ptr, &reader_buffer_len);
        read_len_bytes += reader_buffer_len;
        cur_buffer_ptr += reader_buffer_len;
        cur_writer_ptr += record_num * sizeof(uint32_t);

        // 还原base_key
        uint32_t basekey_num = m_block_header->num_basekeys();
        reader_buffer_len = *r_len - read_len_bytes;
        uint32_t* basekey_ptr = reinterpret_cast<uint32_t*>(cur_writer_ptr);
        DecodeOffsets<uint32_t>(basekey_ptr, basekey_num, cur_buffer_ptr, &reader_buffer_len);
        read_len_bytes += reader_buffer_len;
        cur_buffer_ptr += reader_buffer_len;
        cur_writer_ptr += basekey_num * sizeof(uint32_t);
    }

    // 如果是变长value，需要还原value_offset
    if (val_type == kTypeVariableLen)
    {
        // 还原key_offset
        uint32_t reader_buffer_len = *r_len - read_len_bytes;
        uint32_t* val_offset_ptr = reinterpret_cast<uint32_t*>(cur_writer_ptr);
        DecodeOffsets<uint32_t>(val_offset_ptr, record_num, cur_buffer_ptr, &reader_buffer_len);
        read_len_bytes += reader_buffer_len;
        cur_buffer_ptr += reader_buffer_len;
        cur_writer_ptr += record_num * sizeof(uint32_t);
    }

    *w_len = cur_writer_ptr - w_buffer;
    *r_len = read_len_bytes;
}

/// @brief  固定长度key的查找;
/// @param  key, 查询key的Buffer
/// @param  key_len, 查询key len
/// @param  record_no, 需要查找的record No;
/// @param  base_index, 标识base串record No 的数组下标;
/// @param  flag:
///               kAccurateFind = 0, /// <精确查找，
///               kLowBoundFind = 1  /// <lowbound查找,
/// @param   is_accurated_hit, 在low_bound查找时，返回是否精确命中;
/// @retval  kRetOK:            成功
///          kRetRecordNotFind: Not find
RetCode ReaderBlock::FixedKeyLenFind(const void* key, uint16_t key_len,
                                     uint32_t* record_no, uint32_t* base_index,
                                     uint32_t flag, bool* is_accurated_hit) const
{
    assert(m_block_header->num_records() >= 1);

    RetCode ret = kRetOK;
    *is_accurated_hit = false;
    bool  is_inclusive = false;
    (*base_index) = 0;

    /// 1: 比较最大公共前缀串, 看是否能命中;
    int16_t compare_ret = CompareByteString(key, key_len, m_key_common_prefix,
                                        m_block_header->key_max_common_prefix_len(), &is_inclusive);

    if (compare_ret < 0)
    {
        /// 要查找的串小于最大公共串;
        if (flag == ReaderBlock::kLowBoundFind)
        {
            /// 第一条数据为所查找的数据;
            (*record_no) = 0;
        }
        else
        {
            ret = kRetRecordNotFind;
        }

        return ret;
    }

    if (compare_ret == 0)
    {
        /// 要查找的串等于最大公共串;
        if (m_sstable_header->options().fixed_key_len() ==
            m_block_header->key_max_common_prefix_len())
        {
            *is_accurated_hit = true;
            (*record_no) = 0;
        }
        else
        {
            if (flag == ReaderBlock::kLowBoundFind)  /// 第一条数据为所查找的数据;
            {
                (*record_no) = 0;
            }
            else
            {
                ret = kRetRecordNotFind;
            }
        }

        return ret;
    }

    // 要查找的串大于最大公共串;
    if (is_inclusive)
    {
        /// 4: 要查找的串大于最大公共串, 并且包含最大公共串, 进行二分查找;
        const char* cur_key = static_cast<const char*>(key) + m_block_header->key_max_common_prefix_len();
        uint16_t cur_key_len = key_len - m_block_header->key_max_common_prefix_len();
        bool search_ret = FixedKeyLenBinarySearch(cur_key, cur_key_len,
                          record_no, flag, is_accurated_hit);

        if (!search_ret)
        {
            ret = kRetRecordNotFind;
        }
    }
    else
    {
        /// 5: 要查找的串大于最小公共串, 并且不包含最小公共串;
        ret = kRetRecordNotFind;
    }

    return ret;
}

/// @brief  Key的二分查找;
/// @param  key, key的buffer指针;
/// @param  key_len, key的长度;
/// @param  record_no,  输出该块当中的记录号;
/// @param  flag,
/// @param  is_accurated_hit, 在low_bound查找时，返回是否精确命中;
/// @retval true, 找到;
///         false, 找不到;
bool ReaderBlock::FixedKeyLenBinarySearch(const void* key, uint16_t key_len, uint32_t* record_no,
        uint32_t flag, bool* is_accurated_hit) const
{
    int32_t  begin = 0;
    int32_t  end = m_block_header->num_records() - 1;
    int32_t middle = 0;
    uint16_t cur_key_len = m_sstable_header->options().fixed_key_len()
                           - m_block_header->key_max_common_prefix_len();
    bool     is_found = false;
    int16_t  cmp_result = 0;

    while (begin <= end)
    {
        middle = (begin + end) / 2;
        const char* middle_key = m_key_start_ptr + middle * cur_key_len;
        cmp_result = CompareByteString(key, key_len, middle_key, cur_key_len);

        if (cmp_result < 0)  /// 上半部份;
        {
            end = middle - 1;
        }
        else if (cmp_result == 0)     /// 找到对应key;
        {
            (*record_no) = middle;


            end = middle;
            if (begin == end) // 找到第一个相等的key再跳出
            {
                *is_accurated_hit = true;
                is_found = true;
                break;
            }
        }
        else    /// 下半部分;
        {
            begin = middle + 1;
        }
    }

    if ((!is_found) && (flag == ReaderBlock::kLowBoundFind))
    {
        if (begin <= (m_block_header->num_records() - 1))   /// 比较begin与key的大小;
        {
                (*record_no) = begin;
                is_found = true;
        }
    }

    return is_found;
}

/// @brief   变长度key的查找;
/// @param   key, 查询key的Buffer
/// @param   key_len, 查询key len
/// @param   record_no, 需要查找的record No;
/// @param   base_index, 标识base串record No 的数组下标;
/// @param   flag:
///               kAccurateFind = 0, /// <精确查找，
///               kLowBoundFind = 1  /// <lowbound查找，
/// @param   is_accurated_hit, 在low_bound查找时，返回是否精确命中;
/// @retval  kRetOK:            成功
///          kRetRecordNotFind: Not find
RetCode ReaderBlock::VarKeyLenFind(const void* key, uint16_t key_len,
                                   uint32_t* record_no, uint32_t* base_index,
                                   uint32_t flag, bool* is_accurated_hit) const
{
    RetCode ret = kRetOK;

    int32_t     begin = 0;
    int32_t     end = static_cast<int32_t>(m_block_header->num_records() - 1);
    int32_t    middle = 0; /// middle >= 0;
    uint32_t*   begin_base = m_basekey_index_ptr;
    uint32_t*   middle_base = NULL;
    assert(end >= 0);
    uint32_t*   end_base = std::upper_bound(m_basekey_index_ptr,
                                            m_basekey_index_ptr
                                            + m_block_header->num_basekeys(),
                                            static_cast<int32_t>(end));
    assert(end_base > m_basekey_index_ptr);
    end_base--;

    bool        is_found = false;
    int16_t     cmp_result = 0;

    char*       middle_key = NULL;
    uint16_t    middle_key_len = 0;
    char*       middle_basekey = NULL;
    uint16_t    middle_basekey_len = 0;
    uint16_t    cur_prefix_len = 0;
    uint16_t    skipped_len_bytes = 0;

    *is_accurated_hit = false;

    /// binary search;
    while (begin <= end)
    {
        /// 根据middle的编号查找对应的middle_base; middle>=0;
        middle = static_cast<int32_t>((begin + end) / 2);
        middle_base = std::upper_bound(begin_base, end_base + 1, middle);
        assert(middle_base > begin_base);
        middle_base--;

        if (middle == static_cast<int32_t>(*middle_base))  /// middle串为base串,没有压缩;
        {
            if (middle == 0)
            {
                middle_key = m_key_start_ptr;
                middle_key_len = static_cast<uint16_t>(*m_key_offset_ptr);
            }
            else
            {
                middle_key = m_key_start_ptr + *(m_key_offset_ptr + middle - 1);
                middle_key_len = static_cast<uint16_t>(*(m_key_offset_ptr + middle)
                    - *(m_key_offset_ptr + middle - 1));
            }

            cmp_result = CompareByteString(key, key_len, middle_key, middle_key_len);
        }
        else
        {
            assert(middle > 0);
            middle_key = m_key_start_ptr + *(m_key_offset_ptr + middle - 1);
            middle_key_len = static_cast<uint16_t>(*(m_key_offset_ptr + middle)
                                                   - *(m_key_offset_ptr + middle - 1));

            if (*middle_base == 0)
            {
                middle_basekey = m_key_start_ptr;
                middle_basekey_len = static_cast<uint16_t>(*m_key_offset_ptr);
            }
            else
            {
                middle_basekey = m_key_start_ptr + *(m_key_offset_ptr + *middle_base - 1);
                middle_basekey_len = static_cast<uint16_t>(*(m_key_offset_ptr + *middle_base)
                    - *(m_key_offset_ptr + *middle_base - 1));
            }

            skipped_len_bytes = VariantInteger::Decode(middle_key,
                                middle_key_len,
                                &cur_prefix_len);
            middle_key += skipped_len_bytes;
            middle_key_len -= skipped_len_bytes;

            /// 比较key和前缀串;
            bool is_inclusive = false;
            cmp_result = CompareByteString(key, key_len, middle_basekey,
                cur_prefix_len, &is_inclusive);

            if (cmp_result == 0)
            {
                cmp_result = -1;    // key与prefix相同, 表示key<middle;
            }
            else if (cmp_result > 0 && is_inclusive)   // key包含prefix,需要继续比较剩余的串;
            {
                cmp_result = CompareByteString(static_cast<const char*>(key) + cur_prefix_len,
                                           key_len - cur_prefix_len,
                                           middle_key,
                                           middle_key_len);
            }
        }

        if (cmp_result < 0)   /// 前半部份;
        {
            end = middle - 1;
            end_base = middle_base;  // 只是设立查找base的区域，并不一定是该位置。
        }
        else if (cmp_result == 0)    // 找到对应key;
        {
            (*record_no) = middle;
            (*base_index) = middle_base - m_basekey_index_ptr;

            end = middle;
            end_base = middle_base;  // 只是设立查找base的区域，并不一定是该位置。
            if (begin == end) // 找到第一个相等的key再跳出
            {
                *is_accurated_hit = true;
                is_found = true;
                break;
            }
        }
        else     // 后半部分;
        {
            begin = middle + 1;
            begin_base = middle_base;    // 只是设立查找base的区域，并不一定是该位置。
        }
    }

    if ((!is_found) && (flag == ReaderBlock::kLowBoundFind))
    {
        if (begin <= (m_block_header->num_records() - 1))  /// 比较begin与key的大小;
        {
            // 查找begin的真正位置;
            begin_base = std::upper_bound(begin_base,
                m_basekey_index_ptr + m_block_header->num_basekeys(), begin);

            begin_base--;

           (*record_no)  = begin;
           (*base_index) = begin_base - m_basekey_index_ptr;
           is_found = true;
        }
    }

    if (!is_found) ret = kRetRecordNotFind;

    return ret;
}

} // namespace
