#include "common/file/sstable/sstable_merger.h"
#include "common/file/file.h"

#include "thirdparty/glog/logging.h"

namespace sstable
{
/// 构造和析构函数
/// @brief 外部指定新文件的描述信息(包括, kv_type, block_size, compress_type, bloomfilter);
SSTableMerger::SSTableMerger(const std::vector<std::string>& files,
                             const std::string& output_file,
                             const SSTableOptions& output_file_options,
                             bool need_del_files):
    m_multi_readers(NULL),
    m_writer(NULL),
    m_need_del_files(need_del_files),
    m_mergered_files(files),
    m_output_file(output_file)
{
    m_new_file_options.CopyFrom(output_file_options);
}

/// @brief 对于已经打开的文件进行构造时, 将不会关闭和删除这些文件;
///        外部指定新文件的描述信息(包括, kv_type, block_size, compress_type, bloomfilter);
SSTableMerger::SSTableMerger(const std::vector<SSTableReader*>& readers,
                             const std::string& output_file,
                             const SSTableOptions& output_file_options):
    m_multi_readers(NULL),
    m_writer(NULL),
    m_need_del_files(false),
    m_output_file(output_file)
{
    m_multi_readers = new MultiSSTablesReader(readers);
    m_new_file_options.CopyFrom(output_file_options);
}

SSTableMerger::~SSTableMerger()
{
    delete m_multi_readers;
    delete m_writer;
}

/// @brief   将文件从头开始进行Merge成一个文件。
/// @retval  kRetOK:         写入成功
///          kCompressError: 压缩错误
///          kRetFileOverflow: 文件写满
///          kRetParamError: 单条记录大小超过块大小
///          kRetIOError: IO错误;
RetCode SSTableMerger::DoMerge()
{
    RetCode ret = kRetOK;

    if (m_multi_readers == NULL)
    {
        // 若为文件传入时，先初始化多读对象的实例，并且打开所有文件;
        m_multi_readers = new MultiSSTablesReader(m_mergered_files);
        ret = m_multi_readers->OpenFiles();

        if (ret != kRetOK)  // open里面已经加日志，此处不用加日志;
        {
            delete m_multi_readers;
            m_multi_readers = NULL;
            return ret;
        }
    }

    // 构建SSTableWriter;
    ret = CheckAndConstructWriter();

    if (ret != kRetOK)
    {
        return ret;
    }

    assert(m_writer != NULL);
    // 打开文件;
    ret = m_writer->Open();
    assert(ret == kRetOK);

    BaseIterator* readers_iterator = m_multi_readers->CreateIterator();

    while (!readers_iterator->Done())
    {
        std::string cur_key; // 当前key, key有压缩, 必须要Copy一次;
        char* cur_val = NULL; // 当前val
        uint32_t val_len = 0;
        readers_iterator->GetKey(&cur_key);
        readers_iterator->GetValuePtr(&cur_val, &val_len);
        ret = m_writer->WriteRecord(cur_key.data(),
                                    cur_key.length(),
                                    cur_val,
                                    val_len);

        if (ret != kRetOK)
        {
            // 需要删除掉writer;
            delete m_writer;
            m_writer = NULL;
            return ret;
        }

        readers_iterator->Next();
    }

    delete readers_iterator;

    if (ret == kRetOK)
    {
        // dump 到文件当中;
        ret = m_writer->Close();
        assert(ret == kRetOK);

        if (m_need_del_files)
        {
            // 删除原始文件;
            DelFiles();
        }
    }

    return ret;
}

/// @brief   构建SSTableWriter;
/// @retval  kRetOK, 成功;
///          kRetFileOverflow; 文件会溢出;
RetCode SSTableMerger::CheckAndConstructWriter()
{
    assert(m_multi_readers != NULL);
    const std::vector<SSTableReader*>& readers_ref = m_multi_readers->GetReaders();
    uint64_t num_total_records = 0;

    for (uint32_t count_readers = 0; count_readers < readers_ref.size(); count_readers++)
    {
        SSTableReader* cur_reader = readers_ref[count_readers];
        const SSTableHeader* cur_file_header = cur_reader->GetHeader();
        num_total_records += static_cast<uint64_t>(cur_file_header->num_records());

        if (num_total_records >
            static_cast<uint64_t>(kMaxNumOfRecords)
            * static_cast<uint64_t>(kMaxNumOfRecords))
        {
            LOG(ERROR) << "merger files failed, exceed max record num, cur num:"
                       << num_total_records;

            return kRetFileOverflow;
        }
    }
    if (m_new_file_options.bloomfilter_size() > 0) {
        m_new_file_options.set_bloomfilter_size(num_total_records);
    }

    if (m_writer == NULL)
    {
        m_writer = new SSTableWriter(m_output_file.c_str(),
                                     m_new_file_options);
    }

    return kRetOK;
}

/// @brief   删除合并前的文件;
void SSTableMerger::DelFiles()
{
    assert(m_multi_readers != NULL);
    m_multi_readers->CloseFiles();

    for (uint32_t count_files = 0; count_files < m_mergered_files.size(); count_files++)
    {
        int32_t ret = ::File::Remove(m_mergered_files[count_files].c_str());
        assert(ret == 0);
        (void) ret;
    }

    m_mergered_files.clear();
}

} // namespace
