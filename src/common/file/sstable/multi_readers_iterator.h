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
    // ����
    virtual ~MultiReadersIterator()
    {
        ClearCurHeap();
        // ���iterator;
        for (uint32_t iter_count = 0; iter_count < m_iters.size(); iter_count++)
        {
            delete m_iters[iter_count];
        }
        m_multi_sstables_reader->DecreaseIteratorCount();
    }

    /// @brief   ���¶�λ����ʼλ��;
    virtual void Reset();

    /// @brief   ͬ��˳���ȡ��һ��Record;
    /// @param   should_prefetch, true, ��ʾ��Ҫ���ļ�ʱ��Ҫ�ļ�����Ԥȡ����;
    //           false ��ʾ����Ҫ���ļ�����Ԥȡ����;
    virtual void Next(bool should_prefetch = true);

    /// @brief   ��ȷ��λ��ĳ��Key����ʼλ��,
    ///          �������ļ��ж�û���ҵ�Key, ��iteratorΪ����״̬;
    /// @param   key, ����key��ָ��;
    /// @param   key_len, key�ĳ���;
    virtual void Seek(const void* key, uint32_t key_len);

    /// @brief   low_bound��λ��ĳ��Key����ʼλ��;
    /// @param   key, ����key��ָ��;
    /// @param   key_len, key�ĳ���;
    /// @param   is_accurate_hit, �������, ��ʾ�Ƿ���ȫ����, Ϊ��ʱ��ȡ��ֵ;
    virtual void LowBoundSeek(const void* key,
        uint32_t key_len, bool* is_accurate_hit = NULL);

    /// @brief �жϵ�ǰ��Iterator�Ƿ�ΪEof;
    /// @retval ����;
    virtual bool Done() const;

    /// @brief   ��ȡ��ǰ��¼�Ľ�ѹ�����֮���key
    /// @param   key, �����¼��key��Buffer
    /// @param   key_len, �����¼��key len������ʱΪ�洢key��Buffer����
    /// @retval  kRetOK��       ��ȡ�ɹ�
    ///          kRetBufferOverflow��   ʧ�ܣ����Ȳ���ʱ��key_len ������Ҫ�ĳ���;
    virtual RetCode GetKey(char* key, uint16_t* key_len) const;

    /// @brief   ��ȡ��ǰ��¼�Ľ�ѹ�����֮���key
    /// @param   key, ����������ǰ��¼key��string����
    /// @retval  ��
    virtual void GetKey(std::string* key) const;

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
    /// @param   val, �������Buffer,
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
    /// @param   key_len, ���Key�ĳ���,����ʱΪ�洢key��Buffer����
    /// @param   val, ���Data,
    /// @param   val_len, data�ĳ���, ����ʱΪ�洢���ݵ�Buffer����;
    /// @retval  kRetOK��    �ɹ�;
    ///          kRetBufferOverflow��ʧ�ܣ�Buffer���Ȳ���; ���Ȳ���ʱ,
    ///          key_len, val_len������Ҫ�ĳ���;
    virtual RetCode GetKVPair(char* key,
                      uint16_t* key_len,
                      char* val,
                      uint32_t* val_len) const;

    /// @brief   ��ȡ��ǰ��¼��Key��Value Pair;
    /// @param   key, ����������ǰ��¼key��string����
    /// @param   val, ����������ǰ���ݵ�string����
    /// @retval  ��;
    virtual void  GetKVPair(std::string* key, std::string* val) const;

protected:

    friend class MultiSSTablesReader;

    /// @brief ��������; �˴�ֻ��Copy Heap��Ptr;
    explicit MultiReadersIterator(const MultiReadersIterator& iter_ref):
        m_multi_sstables_reader(iter_ref.m_multi_sstables_reader),
        m_heap(iter_ref.m_heap),
        m_iters(iter_ref.m_iters)
    {}

    /// @brief ��MultiSSTablesReader���õ��ܱ����Ĺ��캯��
    /// @param heap_ptr, heap��ָ��
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

    /// @brief ��յ�ǰ��;
    void ClearCurHeap();

private:

    // ���ڹ�����С�ѵıȽϺ���;
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

    MultiSSTablesReader* m_multi_sstables_reader; // ָ��MultiSSTableReader��ָ��;
    IteratorHeap  m_heap; // iterator�������;
    std::vector<SSTableReaderIterator*> m_iters; // ����iteratorָ������;
};

/// @brief   ���¶�λ����ʼλ��;
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

/// @brief   ͬ��˳���ȡ��һ��Record;
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

/// @brief   ��ȷ��λ��ĳ��Key����ʼλ��,
///          �������ļ��ж�û���ҵ�Key, ��iteratorΪ����״̬;
/// @param   key, ����key��ָ��;
/// @param   key_len, key�ĳ���;
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

/// @brief  low_bound��λ��ĳ��Key����ʼλ��;
/// @param  key, ����key��ָ��;
/// @param  key_len, key�ĳ���;
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

/// @brief �жϵ�ǰ��Iterator�Ƿ�ΪEof;
/// @retval ����;
inline bool MultiReadersIterator::Done() const
{
    return m_heap.empty();
}


/// @brief   ��ȡ��ǰ��¼�Ľ�ѹ�����֮���key
/// @param   key, �����¼��key��Buffer
/// @param   key_len, �����¼��key len������ʱΪ�洢key��Buffer����
/// @retval  kRetOK��       ��ȡ�ɹ�
///          kRetBufferOverflow��   ʧ�ܣ����Ȳ���ʱ��key_len ������Ҫ�ĳ���;
inline RetCode MultiReadersIterator::GetKey(char* key, uint16_t* key_len) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    assert(cur_top_iterator != NULL);
    return cur_top_iterator->GetKey(key, key_len);
}

/// @brief   ��ȡ��ǰ��¼�Ľ�ѹ�����֮���key
/// @param   key, ����������ǰ��¼key��string����
/// @retval  ��
inline void MultiReadersIterator::GetKey(std::string* key) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    assert(cur_top_iterator != NULL);
    return cur_top_iterator->GetKey(key);
}

/// @brief   ��ȡ��ǰ��¼��val��ָ��, ʹ�øú���ʱ��Ҫע�⣺
///           1: �������val=NULLʱ, �൱��ֻȡֵ�ĳ���;
///           2: ��ȡ��ȥ��val��ָ�����Ч�������ں�Iterator��һ��;
///              ��Iterator�����ı�ʱ(����++����֮��), val��ָ������ݿ��ܻ���Ч;
/// @param   val, ��������ֵ��ָ���ָ��;
/// @param   val_len, �����������, �ò�������ΪNULL;
/// @retval  ��;
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


/// @brief   ��ȡ��ǰ��¼��out_val;
/// @param   val, �������Buffer,
///          val_len,������ݵĳ��ȣ� ����ʱΪ�洢���ݵ�Buffer����;
/// @retval  kRetOK��       �ɹ�;
///          kRetBufferOverflow��   ʧ�ܣ����Ȳ���ʱ��val_len ������Ҫ�ĳ���;
inline RetCode MultiReadersIterator::GetValue(char* val, uint32_t* val_len) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    assert(cur_top_iterator != NULL);
    return cur_top_iterator->GetValue(val, val_len);
}

/// @brief   ��ȡ��ǰ��¼��data;
/// @param   val, ����������ǰ���ݵ�string����
/// @retval  ��;
inline void  MultiReadersIterator::GetValue(std::string* val) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    assert(cur_top_iterator != NULL);
    cur_top_iterator->GetValue(val);
}

/// @brief   ��ȡ��ǰ��¼��Key��Value Pair;
/// @param   key, ���Key��
/// @param   key_len, ���Key�ĳ���,����ʱΪ�洢key��Buffer����
///          val, ���Data,
///          val_len, data�ĳ���, ����ʱΪ�洢���ݵ�Buffer����;
/// @retval  kRetOK��    �ɹ�;
///          kRetBufferOverflow��ʧ�ܣ�Buffer���Ȳ���; ���Ȳ���ʱ,
///          key_len, val_len������Ҫ�ĳ���;
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

/// @brief   ��ȡ��ǰ��¼��Key��Value Pair;
/// @param   key, ����������ǰ��¼key��string����
///          val, ����������ǰ���ݵ�string����
/// @retval  ��;
inline void  MultiReadersIterator::GetKVPair(std::string* key, std::string* val) const
{
    assert((m_multi_sstables_reader != NULL) && (!m_heap.empty()));
    SSTableReaderIterator* cur_top_iterator = m_heap.top();
    cur_top_iterator->GetKey(key);
    assert(cur_top_iterator != NULL);
    cur_top_iterator->GetValue(val);
}

/// @brief ��յ�ǰ��;
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
