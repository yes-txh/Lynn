#ifndef SSTABLE_SSTABLE_MERGER_H_
#define SSTABLE_SSTABLE_MERGER_H_

#include <vector>
#include <string>
#include "common/file/sstable/sstable_def.h"
#include "common/file/sstable/sstable_reader.h"
#include "common/file/sstable/sstable_writer.h"
#include "common/file/sstable/sstable_header.pb.h"
#include "common/file/sstable/sstable_reader_iterator.h"
#include "common/file/sstable/multi_sstables_reader.h"
#include "common/file/sstable/multi_readers_iterator.h"

namespace sstable
{
// 同步merger, merger文件当中不处理record删除情况，即所合并的文件当中不含有记录删除信息。
class SSTableMerger
{
public:
    /// @brief 外部指定新文件的描述信息(包括, kv_type, block_size, compress_type, bloomfilter);
    explicit SSTableMerger(const std::vector<std::string>& mergered_files,
                           const std::string& output_file,
                           const SSTableOptions& output_file_options,
                           bool need_del_files = true);

    /// @brief 对于已经打开的文件进行构造时, 将不会关闭和删除这些文件;
    ///        外部指定新文件的描述信息(包括, kv_type, block_size, compress_type, bloomfilter);
    explicit SSTableMerger(const std::vector<SSTableReader*>& mergered_readers,
                           const std::string& output_file,
                           const SSTableOptions& output_file_options);

    ~SSTableMerger();

    /// @brief   将文件从头开始进行Merge成一个文件。
    /// @retval  kRetOK:         写入成功
    ///          kCompressError: 压缩错误
    ///          kRetFileOverflow: 文件写满
    ///          kRetParamError:  单条记录大小超过块大小
    ///          kRetIOError: IO错误;
    RetCode      DoMerge();

private:

    /// @brief   构建SSTableWriter;
    /// @retval  kRetOK, 成功;
    ///          kRetFileOverflow; 文件会溢出;
    RetCode      CheckAndConstructWriter();

    /// @brief   删除合并前的文件;
    void         DelFiles();

    MultiSSTablesReader*      m_multi_readers;        ///< 多路文件读取器的指针;
    SSTableWriter*            m_writer;               ///< writer的指针;
    bool                      m_need_del_files;       ///< 标识是否需要删除文件;
    std::vector<std::string>  m_mergered_files;       ///< merger的原文件文件名;
    std::string               m_output_file;          ///< 输出文件;
    ///< 新文件的创建参数;
    SSTableOptions             m_new_file_options;
};

} // namespace sstable

#endif // SSTABLE_SSTABLE_MERGER_H_
