#ifndef SSTABLE_MULTI_SSTABLES_READER_H_
#define SSTABLE_MULTI_SSTABLES_READER_H_

#include <vector>
#include <string>
#include <queue>

#include "common/system/concurrency/mutex.hpp"
#include "common/file/sstable/sstable_reader.h"
#include "common/file/sstable/base_iterator.h"

namespace sstable
{
class SSTableReader;
class MultiReadersIterator;

// 多个文件的同步读取器, 所读的文件当中不处理record删除记录。
// 若多个文件当中包含有相同key的多个不同值, 则当传入的vector的序列不一样时，
// 相同key的val读取出来的不同值的顺序不一样;
class MultiSSTablesReader
{
public:

    typedef BaseIterator Iterator;

    /// 构造和析构
    explicit MultiSSTablesReader(const std::vector<std::string>& files);
    explicit MultiSSTablesReader(const std::vector<SSTableReader*>& readers);
    ~MultiSSTablesReader();

    /// @brief  打开并且装载所有文件的header,
    ///         index block, bloomfilter, 等信息
    /// @retval kRetOK, 打开成功;
    ///         kRetIOError, IO错误;
    RetCode OpenFiles();

    /// @brief  关闭所有打开的文件;
    RetCode CloseFiles();

    /// @brief  创建一个iterator, 不做定位操作;
    /// @retval iterator的指针;
    BaseIterator* NewIterator();

    /// @brief  创建一个iterator, 并且将iterator的位置定位到第一块记录;
    /// @retval iterator的指针;
    BaseIterator* CreateIterator();

    const std::vector<SSTableReader*>& GetReaders() const
    {
        return m_ptr_readers;
    }

protected:

    friend class MultiReadersIterator;

    // accesser and mutator
    uint32_t GetIteratorCount()
    {
        MutexLocker locker(&m_mutex);
        return m_iterator_count;
    }

    uint32_t IncreaseIteratorCount()
    {
        MutexLocker locker(&m_mutex);
        m_iterator_count++;
        return m_iterator_count;
    }

    uint32_t DecreaseIteratorCount()
    {
        MutexLocker locker(&m_mutex);
        m_iterator_count--;
        return m_iterator_count;
    }

private:

    RecursiveMutex m_mutex;  // 线程锁
    uint32_t m_iterator_count; // iterator上创建出去的iterator的个数;
    std::vector<SSTableReader*>  m_ptr_readers;      // multi reader上所有的sstable reader的指针;
    std::vector<std::string>     m_files_name;       // 文件的文件名;
    bool                         m_need_del_readers; // 判断析构时是否需要删除readers的标识;
    // 以文件名构造该类的实例时需要删除掉所有reader;
    // 以Reader的指针构造时不能删除reader指针;
    // 由外部调用者删除;
}; // class MultiSSTablesReader

} // namespace sstable

#endif // SSTABLE_MULTI_SSTABLES_READER_H_
