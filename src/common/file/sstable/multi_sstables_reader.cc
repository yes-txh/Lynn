#include "common/file/sstable/sstable_def.h"
#include "common/file/sstable/sstable_reader.h"
#include "common/file/sstable/multi_sstables_reader.h"
#include "common/file/sstable/multi_readers_iterator.h"

#include "thirdparty/glog/logging.h"

namespace sstable
{
// 构造和析构
MultiSSTablesReader::MultiSSTablesReader(const std::vector<std::string>& files):
    m_iterator_count(0),
    m_need_del_readers(true)
{
    m_files_name = files;
    m_ptr_readers.reserve(files.size());

    for (uint32_t file_count = 0; file_count < files.size(); file_count++)
    {
        m_ptr_readers[file_count] = NULL;
    }
}

MultiSSTablesReader::MultiSSTablesReader(const std::vector<SSTableReader*>& readers):
    m_iterator_count(0),
    m_need_del_readers(false)
{
    m_ptr_readers = readers;
}

MultiSSTablesReader::~MultiSSTablesReader()
{
    // 检测是否所有iterator都被释放;
    assert(GetIteratorCount() == 0);

    if (m_need_del_readers)
    {
        for (uint32_t reader_count = 0; reader_count < m_ptr_readers.size(); reader_count++)
        {
            delete m_ptr_readers[reader_count];
            m_ptr_readers[reader_count] = NULL;
        }
    }
}

/// @brief  打开并且装载所有文件的header,
///         index block, bloomfilter, 等信息
/// @retval kRetOK, 打开成功;
///         kRetIOError, IO错误;
RetCode MultiSSTablesReader::OpenFiles()
{
    RetCode ret = kRetOK;

    for (uint32_t file_count = 0; file_count < m_files_name.size(); file_count++)
    {
        if (m_ptr_readers[file_count] == NULL)
        {
            SSTableReader* cur_reader = new SSTableReader(m_files_name[file_count].c_str());
            ret = cur_reader->OpenFile();

            if (ret != kRetOK)
            {
                LOG(ERROR) << "open file failed, file name: "
                           << m_files_name[file_count].c_str();
                delete cur_reader;
                m_ptr_readers[file_count] = NULL;
                break;
            }
            else
            {
                LOG(INFO) << "open file succeed, file name: "
                          << m_files_name[file_count].c_str();

                m_ptr_readers.push_back(cur_reader);
            }
        }
    }

    return ret;
}

/// @brief  创建一个iterator, 不做定位操作;
/// @retval iterator的指针;
BaseIterator* MultiSSTablesReader::NewIterator()
{
    BaseIterator* ret_iter = NULL;
    assert(m_ptr_readers.size() > 0);
    if (m_ptr_readers.size() == 1)
    {
        ret_iter = new SSTableReaderIterator(m_ptr_readers[0]);
    }
    else
    {
        ret_iter = new MultiReadersIterator(this);
    }
    IncreaseIteratorCount();
    return ret_iter;
}

/// @brief  创建一个iterator, 并且将iterator的位置定位到第一块记录;
/// @retval iterator的指针;
BaseIterator* MultiSSTablesReader::CreateIterator()
{
    BaseIterator* ret_iter = NULL;
    assert(m_ptr_readers.size() > 0);
    if (m_ptr_readers.size() == 1)
    {
        ret_iter = new SSTableReaderIterator(m_ptr_readers[0]);
        ret_iter->Reset();
    }
    else
    {
        ret_iter = new MultiReadersIterator(this);
        ret_iter->Reset();
    }
    IncreaseIteratorCount();
    return ret_iter;
}


/// @brief  关闭所有打开的文件;
RetCode MultiSSTablesReader::CloseFiles()
{
    RetCode ret = kRetOK;

    if (!m_need_del_readers) return ret;

    for (uint32_t file_count = 0; file_count < m_files_name.size(); file_count++)
    {
        SSTableReader* cur_reader = m_ptr_readers[file_count];

        if (cur_reader != NULL)
        {
            ret = cur_reader->CloseFile();
            assert(ret == kRetOK);
        }
    }

    return ret;
}

} // namespace sstable
