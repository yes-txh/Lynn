#ifndef SSTABLE_SSTABLE_DEF_H_
#define SSTABLE_SSTABLE_DEF_H_

#include "common/base/stdint.h"
#include "common/system/memory/unaligned.hpp"
#include "common/encoding/variant_integer.hpp"
#include "common/base/string/string_algorithm.hpp"
#include "common/compress/block_compression_codec.h"

namespace sstable
{
/// 函数返回状态码定义;
enum RetCode
{
    kRetOK                      = 0,   ///< 返回OK;
    kRetIOError                 = -1,  ///< IO致命错;
    kRetBufferOverflow          = -2,  ///< Buffer不够;
    kRetCompressError           = -3,  ///< 压缩错误;
    kRetUnCompressError         = -4,  ///< 解压错误;
    kRetEof                     = -5,  ///< 文件读完毕;
    kRetBlockOverflow           = -6,  ///< Block写满;
    kRetRecordNotFind           = -7,  ///< Block内未找到记录;
    kRetFileOverflow            = -8,  ///< 文件写满;
    kRetParamError              = -9,  ///< 输入参数错; 单个记录大小小于块大小。
};

/// key和data的类型定义;
enum RecordKVType
{
    kTypeUnknown        = 0x00000000,      ///< 未知类型
    kTypeFixedLen       = 0x00000001,      ///< 固定长度类型
    kTypeVariableLen    = 0x00000002,      ///< 非固定长度类型;

    kKeyTypeMask        = 0x0000000f,     ///< key类型的掩码;
    kValTypeMask        = 0x000000f0,     ///< val类型的掩码;
};


const  uint16_t    kMaxKeyLen = 64 * 1024 - 1;       ///< 最大Key长度,64K长度;
const  uint32_t    kMaxValLen = 32 * 1024 * 1024;    ///< 最大Val长度,32M;
const  uint32_t    kMaxBlockSize = 64 * 1024 * 1024; ///< 最大块大小,64M;
const  uint16_t    kMaxNumOfRecords = 64 * 1024 - 1; ///< 单个块的最大记录个数64K个记录;

/// 压缩类型的定义, 采用block_compression_codec.h中的定义;
typedef uint32_t  CompressType;

/// @brief   取优先级高的压缩算法 压缩算法中优先级, LZO > BMZ > QUICKLZ > GZIP > NONE;
/// @retval  优先级高的压缩算法;
CompressType CompareCompressType(CompressType type1, CompressType type2);

/// @brief   取优先级高的KV Type, 变长优先级 > 定长优先级
/// @retval  优先级高的KV Type,
RecordKVType CompareKVType(RecordKVType type1, RecordKVType type2);

struct  PrefixCompressedKey
{
    PrefixCompressedKey(): remainder_key(NULL),
        remainder_key_len(0),
        prefix_key(NULL),
        prefix_key_len(0)
    {}
    char*       remainder_key;      ///< 剔除掉公共串之后的key指针;
    uint16_t    remainder_key_len;  ///< 剔除掉公共串之后key的长度;
    char*       prefix_key;         ///< prefix key的指针;
    uint16_t    prefix_key_len;     ///< prefix key的长度;
};

///mapreduce用这个结构获取块信息，参见SSTableReader::GetBlockInfos
struct BlockInfo {
    BlockInfo():
        block_no(0),
        start_offset(0),
        length_bytes(0)
    {}
    uint16_t block_no;
    int64_t start_offset;
    int64_t length_bytes;
};

/// @breif  比较两个prefix压缩key的大小;
/// @param  key1,
/// @param  key2,
/// @retval <0: 表示key1<key2;
///         0:  表示key1==key2;
///         >0: 表示key1>key2;
int16_t ComparePrefixCompressedKey(const PrefixCompressedKey& key1,
                                   const PrefixCompressedKey& key2);

/// @breif  压缩sstable_block中存储的offset值信息;
/// @param  writer_buffer, 压缩结果写入的buffer
/// @param  writer_buffer_len, 写入buffer的长度，返回值为已写入的长度
/// @param  reader_buffer, 读取buffer
/// @param  record_num, 要压缩的数目
template<class T>
void EncodeOffsets(char* writer_buffer, uint32_t* writer_buffer_len,
                  T* reader_buffer, uint32_t record_num)
{
    T cur_value;
    uint16_t encoding_size = 0;
    uint32_t total_write_bytes = 0;

    for (uint32_t index = 0; index < record_num; index++)
    {
        if (index > 0) // 计算差值
        {
            cur_value = *(reader_buffer + index) - *(reader_buffer + index - 1);
        }
        else
        {
            cur_value = *(reader_buffer + index);
        }

        encoding_size = static_cast<uint16_t>(VariantInteger::EncodedSize<uint16_t>(cur_value));
        VariantInteger::UncheckedEncode<uint16_t>(cur_value, writer_buffer + total_write_bytes);
        total_write_bytes += encoding_size;
    }
    *writer_buffer_len = total_write_bytes;
}


/// @breif  反解sstable_block中存储的offset值信息;
/// @param  writer_buffer, 反解结果写入的buffer
/// @param  record_num, 要反解的数目
/// @param  reader_buffer, 读取buffer
/// @param  reader_buffer_len, 传入为读取buffer的长度，返回值为已读取的长度
template<class T>
void DecodeOffsets(T* writer_buffer, uint32_t record_num,
                  const void* reader_buffer, uint32_t* reader_buffer_len)
{
    uint32_t skipped_len_bytes = 0;
    uint32_t total_read_buffer = *reader_buffer_len;
    for (uint32_t i = 0; i < record_num; ++i)
    {
        skipped_len_bytes = VariantInteger::Decode(reader_buffer,
                            total_read_buffer, writer_buffer);

        if (i > 0) // 差值还原
        {
            *writer_buffer += *(writer_buffer - 1);
        }

        writer_buffer += 1;
        reader_buffer = static_cast<const char*>(reader_buffer) + skipped_len_bytes;
        total_read_buffer -= skipped_len_bytes;
    }
    *reader_buffer_len -= total_read_buffer;  // 返还的值是reader_buffer中读取的长度
}

} // namespace sstable

#endif // SSTABLE_SSTABLE_DEF_H_
