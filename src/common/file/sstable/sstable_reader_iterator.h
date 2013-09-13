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
    // 析构
    virtual ~SSTableReaderIterator()
    {
        m_reader->ClearBlock(m_block_index);
        m_reader->DecreaseIteratorCount();
    }

    /// @brief   重新定位到起始位置;
    virtual void Reset();

    /// @brief   同步顺序读取下一条Record;
    /// @param   should_prefetch, true, 表示需要读文件时需要文件进行预取操作;
    //           false 表示不需要对文件进行预取操作;
    virtual void Next(bool should_prefetch = true);

    /// @brief   定位到某个Key的起始位置;
    /// @param   key, 输入key的指针;
    /// @param   key_len, key的长度;
    virtual void Seek(const void* key, uint32_t key_len);

    /// @brief   定位到某个Key的起始位置;
    /// @param   key, 输入key;
    virtual void Seek(const std::string& key);

    /// @brief   low_bound定位到某个Key的起始位置;
    /// @param   key, 输入key的指针;
    /// @param   key_len, key的长度;
    /// @param   is_accurate_hit, 输出参数, 表示是否完全命中, 为空时不取该值;
    virtual void LowBoundSeek(const void* key, uint32_t key_len, bool* is_accurate_hit = NULL);

    /// @brief 判断当前的Iterator是否为Eof;
    /// @retval 结束;
    virtual bool Done() const;

    /// @brief   获取当前记录的解压缩完毕之后的key
    /// @param   key, 输出记录的key的Buffer, 当传入的参数为NULL时,
    ///          key_len指向的值必须预付为0, 相当于取Key的长度;
    /// @param   key_len, 输出记录的key len，输入时为存储key的Buffer长度
    /// @retval  kRetOK：       获取成功
    ///          kRetBufferOverflow：   失败，长度不够时，key_len 返回需要的长度;
    virtual RetCode GetKey(char* key, uint16_t* key_len) const;

    /// @brief   获取当前记录的解压缩完毕之后的key
    /// @param   key, 出参数，当前记录key的string类型, 输入不能为NULl,
    /// @retval  无
    virtual void  GetKey(std::string* key) const;

    /// @brief   获取当前记录的val的指针, 使用该函数时需要注意：
    ///           1: 当传入的val=NULL时, 相当于只取值的长度;
    ///           2: 获取出去的val的指针的有效生命周期和Iterator的一致;
    ///              当Iterator发生改变时(例如++操作之后), val的指针的内容可能会无效;
    /// @param   val, 出参数，值的指针的指针;
    /// @param   val_len, 输入输出参数, 该参数不能为NULL;
    /// @retval  无;
    virtual void GetValuePtr(char** val, uint32_t* val_len) const;
    virtual void GetValuePtr(StringPiece* val) const;

    /// @brief   获取当前记录的out_val;
    /// @param   val, 输出数据Buffer, 当传入的参数为NULL时,
    ///          val_len指向的值必须预付为0, 相当于取val的长度;
    /// @param   val_len,输出数据的长度， 输入时为存储数据的Buffer长度;
    /// @retval  kRetOK：       成功;
    ///          kRetBufferOverflow：   失败，长度不够时，val_len 返回需要的长度;
    virtual RetCode GetValue(char* val, uint32_t* val_len) const;

    /// @brief   获取当前记录的data;
    /// @param   val, 出参数，当前数据的string类型
    /// @retval  无;
    virtual void  GetValue(std::string* val) const;

    /// @brief   获取当前记录的Key和Value Pair;
    /// @param   key, 输出Key，
    ///          key_len, 输出Key的长度,输入时为存储key的Buffer长度
    ///          val, 输出Data,
    ///          val_len, data的长度, 输入时为存储数据的Buffer长度;
    /// @retval  kRetOK：    成功;
    ///          kRetBufferOverflow：失败，Buffer长度不够; 长度不够时,
    ///          key_len, val_len返回需要的长度;
    virtual RetCode GetKVPair(char* key, uint16_t* key_len,
                      char* val, uint32_t* val_len) const;

    /// @brief   获取当前记录的Key和Value Pair;
    /// @param   key, 出参数，当前记录key的string类型的指针
    ///          val, 出参数，当前数据的string类型的指针
    /// @retval  无;
    virtual void  GetKVPair(std::string* key, std::string* val) const;

    /// @brief  判断一个iterator的key是否大于另外一个iterator 的key
    /// @param  iter
    /// @retval true, 大于;
    ///         false, 小于或等于;
    bool GreaterThan(const SSTableReaderIterator* iter) const;

    /// @brief  定位iterator到某块的起始位置
    /// @param  block_index, 块号
    /// @retval ture, 成功;
    ///         false, 失败;
    bool JumpToBlock(uint32_t block_index);

    /// @brief  是否读到当前块尾
    /// @retval ture, iterator位于当前块尾;
    ///         false, iterator不位于块尾;
    bool EndOfCurBlock();

protected:

    friend class SSTableReader;
    friend class MultiSSTablesReader;

    // 构造和析构
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

    /// @brief 由SSTableReader调用的构造函数;
    /// @param reader iterator构造时需要
    explicit SSTableReaderIterator(SSTableReader* reader):
        m_reader(reader),
        m_cur_block(NULL),
        m_block_index(kMaxNumOfRecords),
        m_record_index(kMaxNumOfRecords),
        m_basekey_index(kMaxNumOfRecords)
    {}

private:

    SSTableReader*  m_reader;        /// <Iterator所属reader的指针;
    ReaderBlock*    m_cur_block;     /// <当前的Block的指针;
    uint32_t        m_block_index;   /// <当前Block的Block index;
    uint32_t        m_record_index;  /// <当前record所在的块的记录号;
    uint32_t        m_basekey_index; /// <当前record的压缩基准记录的记录在数组中的下标;
};

/// @brief 重新定位到起始位置;
inline void SSTableReaderIterator::Reset()
{
    assert(m_reader != NULL);

    m_reader->ClearBlock(m_block_index);
    m_cur_block = NULL;
    m_block_index = 0;
    m_record_index = 0;
    m_basekey_index = 0;

    // 需要加载第一个块;
    m_cur_block = m_reader->LoadDataBlock(m_block_index);
}

/// @brief 同步顺序读取下一条Record;
inline void SSTableReaderIterator::Next(bool should_prefetch)
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    uint32_t num_records = m_cur_block->GetNumRecords();

    if (m_record_index < (num_records - 1))   /// 本Block还有记录;
    {
        m_basekey_index = m_cur_block->GetNextBaseIndex(m_record_index, m_basekey_index);
        m_record_index++;
        return;
    }

    /// 需要换块;
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

/// @brief   定位到某个Key;
/// @param   key, 输入key的指针;
/// @param   key_len, key的长度;
inline void SSTableReaderIterator::Seek(const void* key, uint32_t key_len)
{
    assert(m_reader != NULL);
    m_reader->ClearBlock(m_block_index);
    m_reader->SeekRecord(key, key_len, &m_cur_block,
                         &m_block_index, &m_record_index, &m_basekey_index);
}

/// @brief   定位到某个Key;
/// @param   key, 输入key;
inline void SSTableReaderIterator::Seek(const std::string& key)
{
    Seek(key.data(), key.length());
}

/// @brief   low_bound定位到某个key;
/// @param   key, 输入key的指针;
/// @param   key_len, key的长度;
/// @param   is_accurate_hit, 输出参数, 为空时不取该值;
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

/// @brief 判断当前的Iterator是否为Eof;
/// @retval 结束;
inline bool SSTableReaderIterator::Done() const
{
    return m_cur_block == NULL ? true : false;
}

/// @brief   获取当前记录的解压缩完毕之后的key
/// @param   key, 输出记录的key的Buffer, 当传入的参数为NULL时,
///          key_len指向的值必须预付为0, 相当于取Key的长度;
/// @param   key_len, 输出记录的key len，输入时为存储key的Buffer长度
/// @retval  kRetOK：       获取成功
///          kRetBufferOverflow：   失败，长度不够时，key_len 返回需要的长度;
inline RetCode SSTableReaderIterator::GetKey(char* key, uint16_t* key_len) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    return m_cur_block->GetKeyByRecordNo(m_record_index, m_basekey_index,
                                         key, key_len);
}

/// @brief   获取当前记录的解压缩完毕之后的key
/// @param   key, 出参数，当前记录key的string类型, 输入不能为NULl,
/// @retval  无
inline void  SSTableReaderIterator::GetKey(std::string* key) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    m_cur_block->GetKeyByRecordNo(m_record_index,
                                  m_basekey_index, key);
}

/// @brief   获取当前记录的val的指针, 使用该函数时需要注意：
///          1: 获取出去的val的指针的有效生命周期和Iterator的一致;
///              当Iterator发生改变时(例如++操作之后), val的指针的内容可能会无效;
/// @param   val, 出参数，值的指针的指针;
/// @param   val_len, 输入输出参数, 该参数不能为NULL;
/// @retval  无;
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

/// @brief   获取当前记录的out_val;
/// @param   val, 输出数据Buffer, 当传入的参数为NULL时,
///          val_len指向的值必须预付为0, 相当于取val的长度;
/// @param   val_len,输出数据的长度， 输入时为存储数据的Buffer长度;
/// @retval  kRetOK：       成功;
///          kRetBufferOverflow：   失败，长度不够时，val_len 返回需要的长度;
inline RetCode SSTableReaderIterator::GetValue(char* val, uint32_t* val_len) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    return m_cur_block->GetValByRecordNo(m_record_index, val, val_len);
}

/// @brief   获取当前记录的data;
/// @param   val, 出参数，当前数据的string类型
/// @retval  无;
inline void  SSTableReaderIterator::GetValue(std::string* val) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    m_cur_block->GetValByRecordNo(m_record_index, val);
}

/// @brief   获取当前记录的Key和Value Pair;
/// @param   key, 输出Key，
///          key_len, 输出Key的长度,输入时为存储key的Buffer长度
///          val, 输出Data,
///          val_len, data的长度, 输入时为存储数据的Buffer长度;
/// @retval  kRetOK：    成功;
///          kRetBufferOverflow：失败，Buffer长度不够; 长度不够时,
///          key_len, val_len返回需要的长度;
inline RetCode SSTableReaderIterator::GetKVPair(char* key, uint16_t* key_len,
        char* val, uint32_t* val_len) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    return m_cur_block->GetKVByRecordNo(m_record_index,
                                        m_basekey_index, key, key_len, val, val_len);
}

/// @brief   获取当前记录的Key和Value Pair;
/// @param   key, 出参数，当前记录key的string类型的指针
///          val, 出参数，当前数据的string类型的指针
/// @retval  无;
inline void  SSTableReaderIterator::GetKVPair(std::string* key, std::string* val) const
{
    assert((m_reader != NULL) && (m_cur_block != NULL));
    m_cur_block->GetKeyByRecordNo(m_record_index, m_basekey_index, key);
    m_cur_block->GetValByRecordNo(m_record_index, val);
}

/// @brief  判断一个iterator的key是否大于另外一个iterator 的key
/// @param  iter
/// @retval true, 大于;
///         false, 小于或等于;
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

/// @brief  定位iterator到某块的起始位置
/// @param  block_index, 块号
/// @retval ture, 成功;
///         false, 失败;
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

/// @brief  是否读到当前块尾
/// @retval ture, iterator位于当前块尾;
///         false, iterator不位于块尾;
inline bool SSTableReaderIterator::EndOfCurBlock()
{
    assert(m_cur_block != NULL);
    return (m_record_index == m_cur_block->GetNumRecords() - 1);
}

} // namespace sstable
#endif // SSTABLE_SSTABLE_READER_ITERATOR_H_
