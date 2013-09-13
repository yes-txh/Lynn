#ifndef SSTABLE_SSTABLE_DEF_H_
#define SSTABLE_SSTABLE_DEF_H_

#include "common/base/stdint.h"
#include "common/system/memory/unaligned.hpp"
#include "common/encoding/variant_integer.hpp"
#include "common/base/string/string_algorithm.hpp"
#include "common/compress/block_compression_codec.h"

namespace sstable
{
/// ��������״̬�붨��;
enum RetCode
{
    kRetOK                      = 0,   ///< ����OK;
    kRetIOError                 = -1,  ///< IO������;
    kRetBufferOverflow          = -2,  ///< Buffer����;
    kRetCompressError           = -3,  ///< ѹ������;
    kRetUnCompressError         = -4,  ///< ��ѹ����;
    kRetEof                     = -5,  ///< �ļ������;
    kRetBlockOverflow           = -6,  ///< Blockд��;
    kRetRecordNotFind           = -7,  ///< Block��δ�ҵ���¼;
    kRetFileOverflow            = -8,  ///< �ļ�д��;
    kRetParamError              = -9,  ///< ���������; ������¼��СС�ڿ��С��
};

/// key��data�����Ͷ���;
enum RecordKVType
{
    kTypeUnknown        = 0x00000000,      ///< δ֪����
    kTypeFixedLen       = 0x00000001,      ///< �̶���������
    kTypeVariableLen    = 0x00000002,      ///< �ǹ̶���������;

    kKeyTypeMask        = 0x0000000f,     ///< key���͵�����;
    kValTypeMask        = 0x000000f0,     ///< val���͵�����;
};


const  uint16_t    kMaxKeyLen = 64 * 1024 - 1;       ///< ���Key����,64K����;
const  uint32_t    kMaxValLen = 32 * 1024 * 1024;    ///< ���Val����,32M;
const  uint32_t    kMaxBlockSize = 64 * 1024 * 1024; ///< �����С,64M;
const  uint16_t    kMaxNumOfRecords = 64 * 1024 - 1; ///< �����������¼����64K����¼;

/// ѹ�����͵Ķ���, ����block_compression_codec.h�еĶ���;
typedef uint32_t  CompressType;

/// @brief   ȡ���ȼ��ߵ�ѹ���㷨 ѹ���㷨�����ȼ�, LZO > BMZ > QUICKLZ > GZIP > NONE;
/// @retval  ���ȼ��ߵ�ѹ���㷨;
CompressType CompareCompressType(CompressType type1, CompressType type2);

/// @brief   ȡ���ȼ��ߵ�KV Type, �䳤���ȼ� > �������ȼ�
/// @retval  ���ȼ��ߵ�KV Type,
RecordKVType CompareKVType(RecordKVType type1, RecordKVType type2);

struct  PrefixCompressedKey
{
    PrefixCompressedKey(): remainder_key(NULL),
        remainder_key_len(0),
        prefix_key(NULL),
        prefix_key_len(0)
    {}
    char*       remainder_key;      ///< �޳���������֮���keyָ��;
    uint16_t    remainder_key_len;  ///< �޳���������֮��key�ĳ���;
    char*       prefix_key;         ///< prefix key��ָ��;
    uint16_t    prefix_key_len;     ///< prefix key�ĳ���;
};

///mapreduce������ṹ��ȡ����Ϣ���μ�SSTableReader::GetBlockInfos
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

/// @breif  �Ƚ�����prefixѹ��key�Ĵ�С;
/// @param  key1,
/// @param  key2,
/// @retval <0: ��ʾkey1<key2;
///         0:  ��ʾkey1==key2;
///         >0: ��ʾkey1>key2;
int16_t ComparePrefixCompressedKey(const PrefixCompressedKey& key1,
                                   const PrefixCompressedKey& key2);

/// @breif  ѹ��sstable_block�д洢��offsetֵ��Ϣ;
/// @param  writer_buffer, ѹ�����д���buffer
/// @param  writer_buffer_len, д��buffer�ĳ��ȣ�����ֵΪ��д��ĳ���
/// @param  reader_buffer, ��ȡbuffer
/// @param  record_num, Ҫѹ������Ŀ
template<class T>
void EncodeOffsets(char* writer_buffer, uint32_t* writer_buffer_len,
                  T* reader_buffer, uint32_t record_num)
{
    T cur_value;
    uint16_t encoding_size = 0;
    uint32_t total_write_bytes = 0;

    for (uint32_t index = 0; index < record_num; index++)
    {
        if (index > 0) // �����ֵ
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


/// @breif  ����sstable_block�д洢��offsetֵ��Ϣ;
/// @param  writer_buffer, ������д���buffer
/// @param  record_num, Ҫ�������Ŀ
/// @param  reader_buffer, ��ȡbuffer
/// @param  reader_buffer_len, ����Ϊ��ȡbuffer�ĳ��ȣ�����ֵΪ�Ѷ�ȡ�ĳ���
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

        if (i > 0) // ��ֵ��ԭ
        {
            *writer_buffer += *(writer_buffer - 1);
        }

        writer_buffer += 1;
        reader_buffer = static_cast<const char*>(reader_buffer) + skipped_len_bytes;
        total_read_buffer -= skipped_len_bytes;
    }
    *reader_buffer_len -= total_read_buffer;  // ������ֵ��reader_buffer�ж�ȡ�ĳ���
}

} // namespace sstable

#endif // SSTABLE_SSTABLE_DEF_H_
