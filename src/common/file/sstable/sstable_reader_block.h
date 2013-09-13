#ifndef SSTABLE_SSTABLE_READER_BLOCK_H_
#define SSTABLE_SSTABLE_READER_BLOCK_H_

#include <assert.h>
#include <string>
#include "common/base/stdint.h"
#include "common/system/concurrency/mutex.hpp"
#include "common/base/string/string_piece.hpp"
#include "common/encoding/variant_integer.hpp"
#include "common/file/sstable/sstable_block.h"
#include "thirdparty/gtest/gtest.h"

namespace sstable
{
class SSTableReader;
class SSTableReaderIterator;
class ReaderBlock: public Block
{
public:

    friend class SSTableReader;

    friend class ReaderBlockTest;
    FRIEND_TEST(ReaderBlockTest, UnCompressBlockData);

    // ���������
    explicit ReaderBlock(const SSTableHeader* header);
    virtual ~ReaderBlock();

    enum
    {
        kAccurateFind = 0,   /// <��ȷ����,
        kLowBoundFind = 1    /// <lowbound����;
    };

    /// accessor

    /// @brief   ��ȡѹ����key; ����iterator�Ƚϴ�Сʱʹ��; ����һ���ڴ�Copy;
    /// @param   record_no, ��¼���±�������;
    /// @param   base_index, ��ʶbase��record No�������±�;
    /// @param   prefix_combine_key, key��ָ��;
    /// @retval
    void GetPrefixCompressedKey(uint32_t record_no, uint32_t base_index,
                                PrefixCompressedKey* prefix_compressed_key) const;

    /// @brief   ����Record No��ȡkey, ��key��ֵCopy��������Buffer����;
    /// @param   record_no, ��¼��
    /// @param   base_index, ��ʶbase��record No�������±�;
    /// @param   key,       ָ��key��Bufferָ��;
    /// @param   key_len,   ����ʱΪ���key��Buffer����, ����ʱΪKey��ʵ�ʳ���;
    /// @retval  kRetOK, �ɹ�;
    ///          kRetBufferOverflow, �ڴ治��;
    RetCode GetKeyByRecordNo(uint32_t record_no, uint32_t base_index,
                             char* key, uint16_t* key_len) const;

    void GetKeyByRecordNo(uint32_t record_no, uint32_t base_index,
                          std::string* key) const;

    /// @brief   ��ȡval��ָ��;
    /// @param   record_no, ��¼��
    /// @param   val,       �������, ���ֵ��Bufferָ���λ�ã�
    /// @param   val_len��  �������, ����ʱΪ���key��Buffer����, ����ʱΪKey��ʵ�ʳ���;
    void GetValPtrByRecordNo(uint32_t record_no, char** val, uint32_t* val_len) const;
    void GetValPtrByRecordNo(uint32_t record_no, StringPiece* val) const;

    /// @brief   ��ȡval;   ��ֵCopy��������Buffer����;
    /// @param   record_no, ��¼��
    /// @param   val,       ���ֵ��Buffer��
    /// @param   val_len��  ����ʱΪ���key��Buffer����, ����ʱΪKey��ʵ�ʳ���;
    /// @retval  kRetOK, �ɹ�;
    ///          kRetBufferOverflow, �ڴ治��;
    RetCode GetValByRecordNo(uint32_t record_no, void* val, uint32_t* val_len) const;
    void GetValByRecordNo(uint32_t record_no, std::string* val) const;

    /// @brief   ��ȡkey,val pair;
    /// @param   record_no, ��¼��
    /// @param   base_index, ��ʶbase��record No�������±�;
    /// @param   key, key��ָ��;
    /// @param   key_len, key�ĳ���;
    /// @param   val, ֵ��ָ��;
    /// @param   val_len; ֵ�ĳ���;
    /// @retval  kRetOK, �ɹ�;
    ///          kRetBufferOverflow, �ڴ治��;
    RetCode GetKVByRecordNo(uint32_t record_no, uint32_t base_index,
                            char* key, uint16_t* key_len,
                            char* val, uint32_t* val_len) const;

    /// @brief   ��ȡ��һ��record��base_index;
    /// @param   cur_record_no, ��ǰ��record_no;
    /// @param   cur_base_index, ��ǰ��base_index;
    /// @retval  record_no+1�ļ�¼��base_index;
    uint32_t GetNextBaseIndex(uint32_t cur_record_no,
                              uint32_t cur_base_index) const;

    /// @brief    ����ĳ��record��key�ڿ��в��ҵ����;
    /// @param    key, ��ѯkey��Buffer
    /// @param    key_len, ��ѯkey len
    /// @param    record_no, ��Ҫ���ҵ�record No;
    /// @param    base_index, ��ʶbase��record No �������±�;
    /// @param    flag: kAccurateFind = 0, /// <��ȷ���ң�
    ///                 kLowBoundFind = 1  /// <lowbound���ң�
    /// @param    is_accurated_hit, ��low_bound����ʱ�������Ƿ�ȷ����;
    /// @retval   kRetOK:            �ɹ�
    ///           kRetRecordNotFind: Not find
    RetCode FindRecordNoByKey(const void* key, uint16_t key_len,
                              uint32_t* record_no, uint32_t* base_index,
                              uint32_t flag, bool* is_accurated_hit) const;

    /// @brief   ���ļ��ж�ȡ���������ݽ�����Block��Buffer����;
    /// @param   buffer, �����buffer
    /// @param   buffer_len, �����buffer len
    /// @retval  kRetOK, �ɹ�;
    ///          kRetUnCompressError, ��ѹʧ��;
    RetCode UnCompressBlockData(const void* buffer, uint32_t buffer_len);

    /// @brief   �����ڴ浱�еķ�ѹ��Block;
    /// @param   buffer, �����buffer;
    /// @param   buffer_len, �����buffer len;
    /// @retval
    void ParseBlockData(const char* buffer, uint32_t buffer_len);

protected:

    /// @brief   ������ü���;
    ///          �����Ѿ�����, ���ü���,
    /// @param   ��
    /// @retval  ���ü�����ֵ;
    uint32_t IncRef()
    {
        m_ref++;
        return m_ref;
    }

    /// @brief   �������ü���;
    ///          �����Ѿ�����, ���ü���,
    /// @param   ��
    /// @retval  ���ü�����ֵ;
    uint32_t DecRef()
    {
        m_ref--;
        return m_ref;
    }

    // accessor and mutator for reader;
    void SetMemoryFromCache()
    {
        m_memory_from_cache = true;
    }

    bool IsMemoryFromCache()
    {
        return m_memory_from_cache;
    }

    /// @brief   ���ó�ԱBufferΪ��,
    ///          ���ҽ�buffer��ָ��ͳ���ȡ��,ע�⡢�˴�����ɾ��Buffer;
    /// @param   buffer;
    /// @retval  ��;
    void SwapOutBuffer(char** buffer, uint32_t* buffer_len)
    {
        assert((m_buffer != NULL) && (m_buffer_len != 0));
        assert((buffer != NULL) && (buffer_len != NULL));
        (*buffer) = m_buffer;
        (*buffer_len) = m_buffer_len;
        m_buffer = NULL;
        m_buffer_len = 0;
    }

private:

    /// @brief  �̶�����key�Ĳ���;
    /// @param  key, ��ѯkey��Buffer
    /// @param  key_len, ��ѯkey len
    /// @param  record_no, ��Ҫ���ҵ�record No;
    /// @param  base_index, ��ʶbase��record No �������±�;
    /// @param  flag:
    ///               kAccurateFind = 0, /// <��ȷ���ң�
    ///               kLowBoundFind = 1  /// <lowbound����,
    /// @param   is_accurated_hit, ��low_bound����ʱ�������Ƿ�ȷ����;
    /// @retval  kRetOK:            �ɹ�
    ///          kRetRecordNotFind: Not find
    RetCode FixedKeyLenFind(const void* key, uint16_t key_len, uint32_t* record_no,
                            uint32_t* base_index, uint32_t flag, bool* is_accurated_hit) const;

    /// @brief  Key�Ķ��ֲ���;
    /// @param  key, key��bufferָ��;
    /// @param  key_len, key�ĳ���;
    /// @param  record_no,  ����ÿ鵱�еļ�¼��;
    /// @param  flag,
    /// @param   is_accurated_hit, ��low_bound����ʱ�������Ƿ�ȷ����;
    /// @retval true, �ҵ�;
    ///         false, �Ҳ���;
    bool FixedKeyLenBinarySearch(const void* key, uint16_t key_len, uint32_t* record_no,
                                 uint32_t flag, bool* is_accurated_hit) const;

    /// @brief   �䳤��key�Ĳ���;
    /// @param   key, ��ѯkey��Buffer
    /// @param   key_len, ��ѯkey len
    /// @param   record_no, ��Ҫ���ҵ�record No;
    /// @param   base_index, ��ʶbase��record No �������±�;
    /// @param   flag:
    ///               kAccurateFind = 0, /// <��ȷ����,
    ///               kLowBoundFind = 1  /// <lowbound����,
    /// @param   is_accurated_hit, ��low_bound����ʱ�������Ƿ�ȷ����;
    /// @retval  kRetOK:            �ɹ�
    ///          kRetRecordNotFind: Not find
    RetCode VarKeyLenFind(const void* key, uint16_t key_len, uint32_t* record_no,
                          uint32_t* base_index, uint32_t flag, bool* is_accurated_hit) const;

    /// @brief   ���ļ��ж�ȡ���������ݽ�����Block��Buffer����;
    /// @param   buffer, �����buffer
    /// @param   buffer_len, �����buffer len
    /// @retval  kRetOK, �ɹ�;
    ///          kRetUnCompressError, ��ѹʧ��;
    RetCode UnCompressBlockHeader(const char* buffer, uint32_t buffer_len);

    /// @brief   ���ļ��ж�ȡ���������ݽ�����Block��Buffer����;
    /// @param   w_buffer, �����buffer
    /// @param   r_buffer, �����buffer
    /// @param   w_len,    �����buffer len
    /// @param   r_len,    �����buffer len
    /// @retval  kRetOK, �ɹ�;
    ///          kRetUnCompressError, ��ѹʧ��;
    void UnCompressOffset(char* w_buffer, const char* r_buffer,
                             uint32_t* w_len, uint32_t* r_len);

    uint32_t        m_ref;      /// <block�����ü���;
    bool            m_memory_from_cache; /// <��ʶ��������ڴ��Ƿ�������Cache;
};

} // namespace sstable

#endif // SSTABLE_SSTABLE_READER_BLOCK_H_

