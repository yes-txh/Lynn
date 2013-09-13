#ifndef SSTABLE_SSTABLE_READER_ITERATOR_H_
#define SSTABLE_SSTABLE_READER_ITERATOR_H_

#include <assert.h>
#include <string>

#include "common/base/stdint.h"
#include "common/base/string/string_piece.hpp"
#include "common/file/sstable/sstable_reader_block.h"
#include "common/file/sstable/sstable_reader.h"
#include "common/file/sstable/block_cache.h"
#include "common/file/sstable/base_iterator.h"

namespace sstable
{
class SSTableReaderIterator : BaseIterator
{
public:
    // ����
    virtual ~SSTableReaderIterator()
    {
        m_reader->ClearBlock(m_block_index);
        m_reader->DecreaseIteratorCount();
    }

    /// @brief   ���¶�λ����ʼλ��;
    virtual void Reset();

    /// @brief   ͬ��˳���ȡ��һ��Record;
    /// @param   should_prefetch, true, ��ʾ��Ҫ���ļ�ʱ��Ҫ�ļ�����Ԥȡ����;
    //           false ��ʾ����Ҫ���ļ�����Ԥȡ����;
    virtual void Next(bool should_prefetch = true);

    /// @brief   ��λ��ĳ��Key����ʼλ��;
    /// @param   key, ����key��ָ��;
    /// @param   key_len, key�ĳ���;
    virtual void Seek(const void* key, uint32_t key_len);

    /// @brief   ��λ��ĳ��Key����ʼλ��;
    /// @param   key, ����key;
    virtual void Seek(const std::string& key);

    /// @brief   low_bound��λ��ĳ��Key����ʼλ��;
    /// @param   key, ����key��ָ��;
    /// @param   key_len, key�ĳ���;
    /// @param   is_accurate_hit, �������, ��ʾ�Ƿ���ȫ����, Ϊ��ʱ��ȡ��ֵ;
    virtual void LowBoundSeek(const void* key, uint32_t key_len, bool* is_accurate_hit = NULL);

    /// @brief �жϵ�ǰ��Iterator�Ƿ�ΪEof;
    /// @retval ����;
    virtual bool Done() const;

    /// @brief   ��ȡ��ǰ��¼�Ľ�ѹ�����֮���key
    /// @param   key, �����¼��key��Buffer, ������Ĳ���ΪNULLʱ,
    ///          key_lenָ���ֵ����Ԥ��Ϊ0, �൱��ȡKey�ĳ���;
    /// @param   key_len, �����¼��key len������ʱΪ�洢key��Buffer����
    /// @retval  kRetOK��       ��ȡ�ɹ�
    ///          kRetBufferOverflow��   ʧ�ܣ����Ȳ���ʱ��key_len ������Ҫ�ĳ���;
    virtual RetCode GetKey(char* key, uint16_t* key_len) const;

    /// @brief   ��ȡ��ǰ��¼�Ľ�ѹ�����֮���key
    /// @param   key, ����������ǰ��¼key��string����, ���벻��ΪNULl,
    /// @retval  ��
    virtual void  GetKey(std::string* key) const;

    /// @brief   ��ȡ��ǰ��¼��val��ָ��, ʹ�øú���ʱ��Ҫע�⣺
    ///           1: �������val=NULLʱ, �൱��ֻȡֵ�ĳ���;
    ///           2: ��ȡ��ȥ��val��ָ�����Ч�������ں�Iterator��һ��;
    ///              ��Iterator�����ı�ʱ(����++����֮��), val��ָ������ݿ��ܻ���Ч;
    /// @param   val, ��������ֵ��ָ���ָ��;
    /// @param   val_len, �����������, �ò�������ΪNULL;
    /// @retval  ��;
    virtual void GetValuePtr(char** val, uint32_t* val_len) const;
    virtual void GetValuePtr(StringPiece* val) const;

    /// @brief   ��ȡ��ǰ��¼��out_val;
    /// @param   val, �������Buffer, ������Ĳ���ΪNULLʱ,
    ///          val_lenָ���ֵ����Ԥ��Ϊ0, �൱��ȡval�ĳ���;
    /// @param   val_len,������ݵĳ��ȣ� ����ʱΪ�洢���ݵ�Buffer����;
    /// @retval  kRetOK��       �ɹ�;
    ///          kRetBufferOverflow��   ʧ�ܣ����Ȳ���ʱ��val_len ������Ҫ�ĳ���;
    virtual RetCode GetValue(char* val, uint32_t* val_len) const;

    /// @brief   ��ȡ��ǰ��¼��data;
    /// @param   val, ����������ǰ���ݵ�string����
    /// @retval  ��;
    virtual void  GetValue(std::string* val) const;

    /// @brief   ��ȡ��ǰ��¼��Key��Value Pair;
    /// @param   key, ���Key��
    ///          key_len, ���Key�ĳ���,����ʱΪ�洢key��Buffer����
    ///          val, ���Data,
    ///          val_len, data�ĳ���, ����ʱΪ�洢���ݵ�Buffer����;
    /// @retval  kRetOK��    �ɹ�;
    ///          kRetBufferOverflow��ʧ�ܣ�Buffer���Ȳ���; ���Ȳ���ʱ,
    ///          key_len, val_len������Ҫ�ĳ���;
    virtual RetCode GetKVPair(char* key, uint16_t* key_len,
                      char* val, uint32_t* val_len) const;

    /// @brief   ��ȡ��ǰ��¼��Key��Value Pair;
    /// @param   key, ����������ǰ��¼key��string���͵�ָ��
    ///          val, ����������ǰ���ݵ�string���͵�ָ��
    /// @retval  ��;
    virtual void  GetKVPair(std::string* key, std::string* val) const;

    /// @brief  �ж�һ��iterator��key�Ƿ��������һ��iterator ��key
    /// @param  iter
    /// @retval true, ����;
    ///         false, С�ڻ����;
    bool GreaterThan(const SSTableReaderIterator* iter) const;

    /// @brief  ��λiterator��ĳ�����ʼλ��
    /// @param  block_index, ���
    /// @retval ture, �ɹ�;
    ///         false, ʧ��;
    bool JumpToBlock(uint32_t block_index);

    /// @brief  �Ƿ������ǰ��β
    /// @retval ture, iteratorλ�ڵ�ǰ��β;
    ///         false, iterator��λ�ڿ�β;
    bool EndOfCurBlock();

protected:

    friend class SSTableReader;
    friend class MultiSSTablesReader;

    // ���������
    SSTableReaderIterator():
        m_reader(NULL),
        m_cur_block(NULL),
        m_block_index(kMaxNumOfRecords),
        m_record_index(kMaxNumOfRecords),
        m_basekey_index(kMaxNumOfRecords)
    {}

    explicit SSTableReaderIterator(const SSTableReaderIterator& iter_ref):
        m_reader(iter_ref.m_reader),
        m_cur_block(iter_ref.m_cur_block),
        m_block_index(iter_ref.m_block_index),
        m_record_index(iter_ref.m_record_index),
        m_basekey_index(iter_ref.m_basekey_index)
    {}

    /// @brief ��SSTableReader���õĹ��캯��;
    /// @param reader iterator����ʱ��Ҫ
    explicit SSTableReaderIterator(SSTableReader* reader):
        m_reader(reader),
        m_cur_block(NULL),
        m_block_index(kMaxNumOfRecords),
        m_record_index(kMaxNumOfRecords),
        m_basekey_index(kMaxNumOfRecords)
    {}

private:

    SSTableReader*  m_reader;        /// <Iterator����reader��ָ��;
    ReaderBlock*    m_cur_block;     /// <��ǰ��Block��ָ��;
    uint32_t        m_block_index;   /// <��ǰBlock��Block index;
    uint32_t        m_record_index;  /// <��ǰrecord���ڵĿ�ļ�¼��;
    uint32_t        m_basekey_index; /// <��ǰrecord��ѹ����׼��¼�ļ�¼�������е��±�;
};

/// @brief ���¶�λ����ʼλ��;
inline void SSTableReaderIterator::Reset()
{
    assert(m_reader != NULL);

    m_reader->ClearBlock(m_block_index);
    m_cur_block = NULL;
    m_block_index = 0;
    m_record_index = 0;
    m_basekey_index = 0;

    // ��Ҫ���ص�һ����;
    m_cur_block = m_reader->LoadDataBlock(m_block_index);
}

/// @brief ͬ��˳���ȡ��һ��Record;
inline void SSTableReaderIterator::Next(bool should_prefetch)
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    uint32_t num_records = m_cur_block->GetNumRecords();

    if (m_record_index < (num_records - 1))   /// ��Block���м�¼;
    {
        m_basekey_index = m_cur_block->GetNextBaseIndex(m_record_index, m_basekey_index);
        m_record_index++;
        return;
    }

    /// ��Ҫ����;
    m_reader->ClearBlock(m_block_index);
    uint32_t num_blocks = m_reader->GetNumBlocks();

    if (m_block_index < (num_blocks - 1))
    {
        m_block_index++;

        if (should_prefetch)
        {
            m_cur_block = m_reader->LoadBlockWithPrefetch(m_block_index);
        }
        else
        {
            m_cur_block = m_reader->LoadDataBlock(m_block_index);
        }
        m_record_index = 0;
        m_basekey_index = 0;
    }
    else
    {
        m_cur_block = NULL;
        m_block_index = kMaxNumOfRecords;
        m_record_index = kMaxNumOfRecords;
        m_basekey_index = kMaxNumOfRecords;
    }
}

/// @brief   ��λ��ĳ��Key;
/// @param   key, ����key��ָ��;
/// @param   key_len, key�ĳ���;
inline void SSTableReaderIterator::Seek(const void* key, uint32_t key_len)
{
    assert(m_reader != NULL);
    m_reader->ClearBlock(m_block_index);
    m_reader->SeekRecord(key, key_len, &m_cur_block,
                         &m_block_index, &m_record_index, &m_basekey_index);
}

/// @brief   ��λ��ĳ��Key;
/// @param   key, ����key;
inline void SSTableReaderIterator::Seek(const std::string& key)
{
    Seek(key.data(), key.length());
}

/// @brief   low_bound��λ��ĳ��key;
/// @param   key, ����key��ָ��;
/// @param   key_len, key�ĳ���;
/// @param   is_accurate_hit, �������, Ϊ��ʱ��ȡ��ֵ;
inline void SSTableReaderIterator::LowBoundSeek(const void* key,
        uint32_t key_len,
        bool* is_accurate_hit)
{
    assert(m_reader != NULL);
    m_reader->ClearBlock(m_block_index);

    bool b_accurate_hit = false;
    m_reader->LowBoundSeekRecord(key, key_len, &m_cur_block, &m_block_index,
                                 &m_record_index, &m_basekey_index, &b_accurate_hit);

    if (is_accurate_hit != NULL)
    {
        *is_accurate_hit = b_accurate_hit;
    }
}

/// @brief �жϵ�ǰ��Iterator�Ƿ�ΪEof;
/// @retval ����;
inline bool SSTableReaderIterator::Done() const
{
    return m_cur_block == NULL ? true : false;
}

/// @brief   ��ȡ��ǰ��¼�Ľ�ѹ�����֮���key
/// @param   key, �����¼��key��Buffer, ������Ĳ���ΪNULLʱ,
///          key_lenָ���ֵ����Ԥ��Ϊ0, �൱��ȡKey�ĳ���;
/// @param   key_len, �����¼��key len������ʱΪ�洢key��Buffer����
/// @retval  kRetOK��       ��ȡ�ɹ�
///          kRetBufferOverflow��   ʧ�ܣ����Ȳ���ʱ��key_len ������Ҫ�ĳ���;
inline RetCode SSTableReaderIterator::GetKey(char* key, uint16_t* key_len) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    return m_cur_block->GetKeyByRecordNo(m_record_index, m_basekey_index,
                                         key, key_len);
}

/// @brief   ��ȡ��ǰ��¼�Ľ�ѹ�����֮���key
/// @param   key, ����������ǰ��¼key��string����, ���벻��ΪNULl,
/// @retval  ��
inline void  SSTableReaderIterator::GetKey(std::string* key) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    m_cur_block->GetKeyByRecordNo(m_record_index,
                                  m_basekey_index, key);
}

/// @brief   ��ȡ��ǰ��¼��val��ָ��, ʹ�øú���ʱ��Ҫע�⣺
///          1: ��ȡ��ȥ��val��ָ�����Ч�������ں�Iterator��һ��;
///              ��Iterator�����ı�ʱ(����++����֮��), val��ָ������ݿ��ܻ���Ч;
/// @param   val, ��������ֵ��ָ���ָ��;
/// @param   val_len, �����������, �ò�������ΪNULL;
/// @retval  ��;
inline void SSTableReaderIterator::GetValuePtr(char** val, uint32_t* val_len) const
{
    StringPiece sp_val;
    GetValuePtr(&sp_val);
    if (val != NULL)
    {
        *val = const_cast<char*>(sp_val.data());
    }
    *val_len = sp_val.length();
}

inline void SSTableReaderIterator::GetValuePtr(StringPiece* val) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    return m_cur_block->GetValPtrByRecordNo(m_record_index, val);
}

/// @brief   ��ȡ��ǰ��¼��out_val;
/// @param   val, �������Buffer, ������Ĳ���ΪNULLʱ,
///          val_lenָ���ֵ����Ԥ��Ϊ0, �൱��ȡval�ĳ���;
/// @param   val_len,������ݵĳ��ȣ� ����ʱΪ�洢���ݵ�Buffer����;
/// @retval  kRetOK��       �ɹ�;
///          kRetBufferOverflow��   ʧ�ܣ����Ȳ���ʱ��val_len ������Ҫ�ĳ���;
inline RetCode SSTableReaderIterator::GetValue(char* val, uint32_t* val_len) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    return m_cur_block->GetValByRecordNo(m_record_index, val, val_len);
}

/// @brief   ��ȡ��ǰ��¼��data;
/// @param   val, ����������ǰ���ݵ�string����
/// @retval  ��;
inline void  SSTableReaderIterator::GetValue(std::string* val) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    m_cur_block->GetValByRecordNo(m_record_index, val);
}

/// @brief   ��ȡ��ǰ��¼��Key��Value Pair;
/// @param   key, ���Key��
///          key_len, ���Key�ĳ���,����ʱΪ�洢key��Buffer����
///          val, ���Data,
///          val_len, data�ĳ���, ����ʱΪ�洢���ݵ�Buffer����;
/// @retval  kRetOK��    �ɹ�;
///          kRetBufferOverflow��ʧ�ܣ�Buffer���Ȳ���; ���Ȳ���ʱ,
///          key_len, val_len������Ҫ�ĳ���;
inline RetCode SSTableReaderIterator::GetKVPair(char* key, uint16_t* key_len,
        char* val, uint32_t* val_len) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    return m_cur_block->GetKVByRecordNo(m_record_index,
                                        m_basekey_index, key, key_len, val, val_len);
}

/// @brief   ��ȡ��ǰ��¼��Key��Value Pair;
/// @param   key, ����������ǰ��¼key��string���͵�ָ��
///          val, ����������ǰ���ݵ�string���͵�ָ��
/// @retval  ��;
inline void  SSTableReaderIterator::GetKVPair(std::string* key, std::string* val) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    m_cur_block->GetKeyByRecordNo(m_record_index, m_basekey_index, key);
    m_cur_block->GetValByRecordNo(m_record_index, val);
}

/// @brief  �ж�һ��iterator��key�Ƿ��������һ��iterator ��key
/// @param  iter
/// @retval true, ����;
///         false, С�ڻ����;
inline bool SSTableReaderIterator::GreaterThan(const SSTableReaderIterator* iter) const
{
    assert((m_reader != NULL) && (iter->m_reader != NULL));
    assert((m_cur_block != NULL) && (iter->m_cur_block != NULL));

    sstable::PrefixCompressedKey cur_key;
    sstable::PrefixCompressedKey target_key;

    m_cur_block->GetPrefixCompressedKey(m_record_index, m_basekey_index, &cur_key);
    (iter->m_cur_block)->GetPrefixCompressedKey(iter->m_record_index,
            iter->m_basekey_index, &target_key);

    int prefix_comp_ret = sstable::ComparePrefixCompressedKey(cur_key, target_key);
    return prefix_comp_ret > 0 ? true : false;
}

/// @brief  ��λiterator��ĳ�����ʼλ��
/// @param  block_index, ���
/// @retval ture, �ɹ�;
///         false, ʧ��;
inline bool SSTableReaderIterator::JumpToBlock(uint32_t block_index)
{
    assert(m_reader != NULL);

    m_reader->ClearBlock(m_block_index);
    m_block_index = block_index;
    m_cur_block = m_reader->LoadBlockWithPrefetch(m_block_index);

    if (m_cur_block != NULL)
    {
        m_record_index = 0;
        m_basekey_index = 0;
        return true;
    }
    else
    {
        m_block_index = kMaxNumOfRecords;
        m_record_index = kMaxNumOfRecords;
        m_basekey_index = kMaxNumOfRecords;
        return false;
    }
}

/// @brief  �Ƿ������ǰ��β
/// @retval ture, iteratorλ�ڵ�ǰ��β;
///         false, iterator��λ�ڿ�β;
inline bool SSTableReaderIterator::EndOfCurBlock()
{
    assert(m_cur_block != NULL);
    return (m_record_index == m_cur_block->GetNumRecords() - 1);
}

} // namespace sstable
#endif // SSTABLE_SSTABLE_READER_ITERATOR_H_
