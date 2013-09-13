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
// ͬ��merger, merger�ļ����в�����recordɾ������������ϲ����ļ����в����м�¼ɾ����Ϣ��
class SSTableMerger
{
public:
    /// @brief �ⲿָ�����ļ���������Ϣ(����, kv_type, block_size, compress_type, bloomfilter);
    explicit SSTableMerger(const std::vector<std::string>& mergered_files,
                           const std::string& output_file,
                           const SSTableOptions& output_file_options,
                           bool need_del_files = true);

    /// @brief �����Ѿ��򿪵��ļ����й���ʱ, ������رպ�ɾ����Щ�ļ�;
    ///        �ⲿָ�����ļ���������Ϣ(����, kv_type, block_size, compress_type, bloomfilter);
    explicit SSTableMerger(const std::vector<SSTableReader*>& mergered_readers,
                           const std::string& output_file,
                           const SSTableOptions& output_file_options);

    ~SSTableMerger();

    /// @brief   ���ļ���ͷ��ʼ����Merge��һ���ļ���
    /// @retval  kRetOK:         д��ɹ�
    ///          kCompressError: ѹ������
    ///          kRetFileOverflow: �ļ�д��
    ///          kRetParamError:  ������¼��С�������С
    ///          kRetIOError: IO����;
    RetCode      DoMerge();

private:

    /// @brief   ����SSTableWriter;
    /// @retval  kRetOK, �ɹ�;
    ///          kRetFileOverflow; �ļ������;
    RetCode      CheckAndConstructWriter();

    /// @brief   ɾ���ϲ�ǰ���ļ�;
    void         DelFiles();

    MultiSSTablesReader*      m_multi_readers;        ///< ��·�ļ���ȡ����ָ��;
    SSTableWriter*            m_writer;               ///< writer��ָ��;
    bool                      m_need_del_files;       ///< ��ʶ�Ƿ���Ҫɾ���ļ�;
    std::vector<std::string>  m_mergered_files;       ///< merger��ԭ�ļ��ļ���;
    std::string               m_output_file;          ///< ����ļ�;
    ///< ���ļ��Ĵ�������;
    SSTableOptions             m_new_file_options;
};

} // namespace sstable

#endif // SSTABLE_SSTABLE_MERGER_H_
