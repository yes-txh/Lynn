#ifndef SSTABLE_MULTI_READERS_ITERATOR_H_
#define SSTABLE_MULTI_READERS_ITERATOR_H_

#include <string>
#include <vector>
#include <queue>

#include "common/base/string/string_piece.hpp"
#include "common/file/sstable/multi_sstables_reader.h"
#include "common/file/sstable/sstable_reader_iterator.h"
#include "common/file/sstable/base_iterator.h"

namespace sstable
{
class MultiReadersIterator :public BaseIterator
{
public:
    // 析构
    virtual ~MultiReadersIterator()
    {
        ClearCurHeap();
        // 清空iterator;
        for (uint32_t iter_count = 0; iter_count < m_iters.size(); iter_count++)
        {
            delete m_iters[iter_count];
        }
        m_multi_sstables_reader->DecreaseIteratorCount();
    }

    /// @brief   重新定位到起始位置;
    virtual void Reset();

    /// @brief   同步顺序读取下一条Record;
    /// @param   should_prefetch, true, 表示需要读文件时需要文件进行预取操作;
    //           false 表示不需要对文件进行预取操作;
    virtual void Next(bool should_prefetch = true);

    /// @brief   精确定位到某个Key的起始位置,
    ///          若所有文件中都没有找到Key, 则iterator为结束状态;
    /// @param   key, 输入key的指针;
    /// @param   key_len, key的长度;
    virtual void Seek(const void* key, uint32_t key_len);

    /// @brief   low_bound定位到某个Key的起始位置;
    /// @param   key, 输入key的指针;
    /// @param   key_len, key的长度;
    /// @param   is_accurate_hit, 输出参数, 表示是否完全命中, 为空时不取该值;
    virtual void LowBoundSeek(const void* key,
        uint32_t key_len, bool* is_accurate_hit = NULL);

    /// @brief 判断当前的Iterator是否为Eof;
    /// @retval 结束;
    virtual bool Done() const;

    /// @brief   获取当前记录的解压缩完毕之后的key
    /// @param   key, 输出记录的key的Buffer
    /// @param   key_len, 输出记录的key len，输入时为存储key的Buffer长度
    /// @retval  kRetOK：       获取成功
    ///          kRetBufferOverflow：   失败，长度不够时，key_len 返回需要的长度;
    virtual RetCode GetKey(char* key, uint16_t* key_len) const;

    /// @brief   获取当前记录的解压缩完毕之后的key
    /// @param   key, 出参数，当前记录key的string类型
    /// @retval  无
    virtual void GetKey(std::string* key) const;

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
    /// @param   val, 输出数据Buffer,
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
    /// @param   key_len, 输出Key的长度,输入时为存储key的Buffer长度
    /// @param   val, 输出Data,
    /// @param   val_len, data的长度, 输入时为存储数据的Buffer长度;
    /// @retval  kRetOK：    成功;
    ///          kRetBufferOverflow：失败，Buffer长度不够; 长度不够时,
    ///          key_len, val_len返回需要的长度;
    virtual RetCode GetKVPair(char* key,
                      uint16_t* key_len,
                      char* val,
                      uint32_t* val_len) const;

    /// @brief   获取当前记录的Key和Value Pair;
    /// @param   key, 出参数，当前记录key的string类型
    /// @param   val, 出参数，当前数据的string类型
    /// @retval  无;
    virtual void  GetKVPair(std::string* key, std::string* val) const;

protected:

    friend class MultiSSTablesReader;

    /// @brief 拷贝构造; 此处只是Copy Heap的Ptr;
    explicit MultiReadersIterator(const MultiReadersIterator& iter_ref):
        m_multi_sstables_reader(iter_ref.m_multi_sstables_reader),
        m_heap(iter_ref.m_heap),
        m_iters(iter_ref.m_iters)
    {}

    /// @brief 由MultiSSTablesReader调用的受保护的构造函数
    /// @param heap_ptr, heap的指针
    explicit MultiReadersIterator(MultiSSTablesReader* sstables_reader):
        m_multi_sstables_reader(sstables_reader)
    {
        const std::vector<SSTableReader*>& readers = sstables_reader->GetReaders();
        uint32_t num_readers = readers.size();
        m_iters.reserve(num_readers);
        for (uint32_t iter_count = 0; iter_count < num_readers; iter_count++) {
             SSTableReader* cur_reader = readers[iter_count];
             SSTableReaderIterator* cur_iter = cur_reader->NewIterator();
             m_iters.push_back(cur_iter);
        }
    }

    /// @brief 清空当前堆;
    void ClearCurHeap();

private:

    // 用于构建最小堆的比较函数;
    struct CachedUnitPtrCmp
    {
        bool operator()(const SSTableReaderIterator* iter1,
                        const SSTableReaderIterator* iter2) const
        {
            assert((iter1 != NULL) && (iter1 != NULL));
            return iter1->GreaterThan(iter2);
        }
    };

    typedef std::priority_queue < SSTableReaderIterator*,
            std::vector<SSTableReaderIterator*>,
            CachedUnitPtrCmp > IteratorHeap;

    MultiSSTablesReader* m_multi_sstables_reader; // 指向MultiSSTableReader的指针;
    IteratorHeap  m_heap; // iterator的排序表;
    std::vector<SSTableReaderIterator*> m_iters; // 备用iterator指针数组;
};

/// @brief   重新定位到起始位置;
inline void MultiReadersIterator::Reset()
{
    ClearCurHeap();
    for (uint32_t iter_count = 0; iter_count < m_iters.size(); iter_count++)
    {
        SSTableReaderIterator* cur_iter = m_iters[iter_count];
        cur_iter->Reset();
        if (!cur_iter->Done())
        {
            m_heap.push(cur_iter);
        }
    }
}

/// @brief   同步顺序读取下一条Record;
inline void MultiReadersIterator::Next(bool should_prefetch)
{
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    m_heap.pop();

    assert(cur_top_iterator != NULL);
    cur_top_iterator->Next(should_prefetch);

    if (!cur_top_iterator->Done())
    {
        m_heap.push(cur_top_iterator);
    }
}

/// @brief   精确定位到某个Key的起始位置,
///          若所有文件中都没有找到Key, 则iterator为结束状态;
/// @param   key, 输入key的指针;
/// @param   key_len, key的长度;
inline void MultiReadersIterator::Seek(const void* key, uint32_t key_len)
{
    ClearCurHeap();

    bool  is_accurate_hit = false;

    for (uint32_t iter_count = 0; iter_count < m_iters.size(); iter_count++)
    {
        SSTableReaderIterator* cur_iter = m_iters[iter_count];
        if (is_accurate_hit)
        {
            cur_iter->LowBoundSeek(key, key_len);
        }
        else
        {
            cur_iter->LowBoundSeek(key, key_len, &is_accurate_hit);
        }

        if (!cur_iter->Done())
        {
            m_heap.push(cur_iter);
        }
    }
}

/// @brief  low_bound定位到某个Key的起始位置;
/// @param  key, 输入key的指针;
/// @param  key_len, key的长度;
inline void MultiReadersIterator::LowBoundSeek(const void* key,
                                               uint32_t key_len,
                                               bool* is_accurate_hit)
{
    ClearCurHeap();

    for (uint32_t iter_count = 0; iter_count < m_iters.size(); iter_count++)
    {
        SSTableReaderIterator* cur_iter = m_iters[iter_count];
        cur_iter->LowBoundSeek(key, key_len);

        if (!cur_iter->Done())
        {
            m_heap.push(cur_iter);
        }
    }
}

/// @brief 判断当前的Iterator是否为Eof;
/// @retval 结束;
inline bool MultiReadersIterator::Done() const
{
    return m_heap.empty();
}


/// @brief   获取当前记录的解压缩完毕之后的key
/// @param   key, 输出记录的key的Buffer
/// @param   key_len, 输出记录的key len，输入时为存储key的Buffer长度
/// @retval  kRetOK：       获取成功
///          kRetBufferOverflow：   失败，长度不够时，key_len 返回需要的长度;
inline RetCode MultiReadersIterator::GetKey(char* key, uint16_t* key_len) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    assert(cur_top_iterator != NULL);
    return cur_top_iterator->GetKey(key, key_len);
}

/// @brief   获取当前记录的解压缩完毕之后的key
/// @param   key, 出参数，当前记录key的string类型
/// @retval  无
inline void MultiReadersIterator::GetKey(std::string* key) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    assert(cur_top_iterator != NULL);
    return cur_top_iterator->GetKey(key);
}

/// @brief   获取当前记录的val的指针, 使用该函数时需要注意：
///           1: 当传入的val=NULL时, 相当于只取值的长度;
///           2: 获取出去的val的指针的有效生命周期和Iterator的一致;
///              当Iterator发生改变时(例如++操作之后), val的指针的内容可能会无效;
/// @param   val, 出参数，值的指针的指针;
/// @param   val_len, 输入输出参数, 该参数不能为NULL;
/// @retval  无;
inline void MultiReadersIterator::GetValuePtr(char** val, uint32_t* val_len) const
{
    StringPiece sp_val;
    GetValuePtr(&sp_val);
    if (val != NULL)
    {
        *val = const_cast<char*>(sp_val.data());
    }
    *val_len = sp_val.length();
}

inline void MultiReadersIterator::GetValuePtr(StringPiece* val) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    assert(cur_top_iterator != NULL);
    cur_top_iterator->GetValuePtr(val);
}


/// @brief   获取当前记录的out_val;
/// @param   val, 输出数据Buffer,
///          val_len,输出数据的长度， 输入时为存储数据的Buffer长度;
/// @retval  kRetOK：       成功;
///          kRetBufferOverflow：   失败，长度不够时，val_len 返回需要的长度;
inline RetCode MultiReadersIterator::GetValue(char* val, uint32_t* val_len) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    assert(cur_top_iterator != NULL);
    return cur_top_iterator->GetValue(val, val_len);
}

/// @brief   获取当前记录的data;
/// @param   val, 出参数，当前数据的string类型
/// @retval  无;
inline void  MultiReadersIterator::GetValue(std::string* val) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    assert(cur_top_iterator != NULL);
    cur_top_iterator->GetValue(val);
}

/// @brief   获取当前记录的Key和Value Pair;
/// @param   key, 输出Key，
/// @param   key_len, 输出Key的长度,输入时为存储key的Buffer长度
///          val, 输出Data,
///          val_len, data的长度, 输入时为存储数据的Buffer长度;
/// @retval  kRetOK：    成功;
///          kRetBufferOverflow：失败，Buffer长度不够; 长度不够时,
///          key_len, val_len返回需要的长度;
inline RetCode MultiReadersIterator::GetKVPair(char* key,
        uint16_t* key_len,
        char* val,
        uint32_t* val_len) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    assert(cur_top_iterator != NULL);
    return cur_top_iterator->GetKVPair(key, key_len, val, val_len);
}

/// @brief   获取当前记录的Key和Value Pair;
/// @param   key, 出参数，当前记录key的string类型
///          val, 出参数，当前数据的string类型
/// @retval  无;
inline void  MultiReadersIterator::GetKVPair(std::string* key, std::string* val) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    cur_top_iterator->GetKey(key);
    assert(cur_top_iterator != NULL);
    cur_top_iterator->GetValue(val);
}

/// @brief 清空当前堆;
inline void MultiReadersIterator::ClearCurHeap()
{
    while (!m_heap.empty())
    {
        SSTableReaderIterator* top_iter = m_heap.top();
        assert(top_iter != NULL);
        (void) top_iter;
        m_heap.pop();
    }
}

} // namespace sstable

#endif // SSTABLE_MULTI_READERS_ITERATOR_H_
