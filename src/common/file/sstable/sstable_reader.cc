#include "common/collection/bloom_filter.hpp"
#include "common/base/byte_order.hpp"
#include "common/file/file.h"
#include "common/file/sstable/sstable_block.h"
#include "common/file/sstable/sstable_reader_iterator.h"
#include "common/file/sstable/sstable_reader.h"
#include "common/file/sstable/block_cache.h"

#include "thirdparty/glog/logging.h"

namespace sstable
{
SSTableReader::~SSTableReader()
{
    delete [] m_file_read_buffer;

    RetCode close_ret = CloseFile();
    assert(close_ret == kRetOK);
    (void) close_ret;
    // ����Ƿ������Ѿ�������Iterator���Ѿ���ɾ����;
    assert(GetIteratorCount() == 0);

    if (m_blocks != NULL)
    {
        assert(m_index_block != NULL);
        uint32_t num_blocks = m_index_block->GetNumRecords();

        for (uint32_t block_count = 0; block_count < num_blocks; block_count++)
        {
            /// ��Debugģʽ�µ���, Releaseģʽ�������;
            assert(!CheckBlockRef(block_count));
            ClearBlock(block_count);
        }

        delete [] m_blocks;
        m_blocks = NULL;
    }

    delete m_sstable_header;
    delete m_index_block;
    delete m_bloomfilter;
}

/// @brief  ͬ����һ��sstable �ļ�,
///         �����schema, block index, bloomfilter;
/// @param  file_path, �ļ���·��;
/// @retval kRetOK:      �ɹ�
///         kRetIOError:  ���ļ�ʧ��
RetCode SSTableReader::OpenFile()
{
    RetCode ret = kRetOK;

    VLOG(0) << "open file, file name:" << m_file_name.c_str();

    // 1:���ļ�;
    if (m_file == NULL)
    {
        m_file = File::Open(m_file_name.c_str(), File::ENUM_FILE_OPEN_MODE_R);
    }

    if (m_file == NULL)
    {
        LOG(ERROR) << "open file failed, file name:"
                   << m_file_name.c_str();
        return kRetIOError;
    }

    // 2:����Schema;
    ret = LoadSSTableHeader();

    if (ret != kRetOK)
    {
        LOG(ERROR) << "load schema failed, file name:"
                   << m_file_name.c_str();
        return ret;
    }

    // 3:����Index;
    ret = LoadIndex();

    if (ret != kRetOK)
    {
        LOG(ERROR) << "load index failed, file name:"
                   << m_file_name.c_str();
        return ret;
    }

    // 4:����Bloomfilter
    if (m_sstable_header->has_bloomfilter_block())
    {
        ret = LoadBloomfilter();
    }

    if (ret != kRetOK)
    {
        LOG(ERROR) << "load bloomfilter failed, file name:"
                   << m_file_name.c_str();
        return ret;
    }

    const uint32_t num_blocks = m_index_block->GetNumRecords();

    // 6: ��ʼ��m_blocks;
    m_blocks = new ReaderBlock*[num_blocks];
    for (uint32_t i = 0; i < num_blocks; ++i)
    {
        m_blocks[i] = NULL;
    }

    return ret;
}

/// @brief  ͬ���ر�һ��sstable�ļ�;
/// @param  �ļ������ָ��;
/// @retval kRetOK:       �ɹ�;
///         kRetIOError:  �ر��ļ�ʧ��;
RetCode SSTableReader::CloseFile()
{
    RetCode ret = kRetOK;

    if (m_file != NULL)
    {
        uint32_t error_no = ERR_FILE_OK;
        m_file->Close(&error_no);

        if (error_no != ERR_FILE_OK)
        {
            LOG(ERROR) << "close file failed, file name:"
                       << m_file_name.c_str()
                       << "error no:" << error_no;
            ret = kRetIOError;
        }
        else
        {
            VLOG(0) << "close file, file name:" << m_file_name.c_str();
            m_file = NULL;
        }
    }

    return ret;
}

/// @brief  ͬ����һ��sstable�ļ�������map���ڴ浱��;
/// @param  ��
/// @retval kRetOK:  ��ȷ��
///         kRetIOError: IO����;
RetCode SSTableReader::MemMap()
{
    RetCode ret = kRetOK;

    assert(m_index_block != NULL);
    uint32_t  num_blocks = m_index_block->GetNumRecords();
    uint32_t  num_loaded_blocks = 0;

    while (num_loaded_blocks < num_blocks)
    {
        ReaderBlock* cur_block = this->LoadBlockWithPrefetch(num_loaded_blocks);

        if (cur_block == NULL)
        {
            LOG(ERROR) << "memmap failed, file name: "
                       << m_file_name.c_str()
                       << "cur loading block no:"
                       << num_loaded_blocks;
            ret = kRetIOError;
            break;
        }
        else
        {
            num_loaded_blocks++;
        }
    }

    m_memmap_called = true;

    return ret;
}

/// @brief  ����һ��iterator, ������λ����;
/// @retval iterator��ָ��;
SSTableReader::Iterator* SSTableReader::NewIterator()
{
    IncreaseIteratorCount();
    return new Iterator(this);
}

/// @brief  ����һ��iterator, ���ҽ�iterator��λ�ö�λ����һ���¼;
/// @retval iterator��ָ��;
SSTableReader::Iterator* SSTableReader::CreateIterator()
{
    Iterator* ret_iter = new Iterator(this);
    ret_iter->Reset();
    IncreaseIteratorCount();
    return ret_iter;
}

/// @brief  ͨ��Key���ҵ�һ��ֵ, �Ե���sstable�������ȡ,
///         �ú������ڵ������Ѿ�����洢val���ڴ�ʱʹ�á�
/// @param  key, ����key;
/// @param  key_len, ����key len;
/// @param  val, ���ڴ洢first_value��buffer��ָ��;
/// @param  val_len, ����ʱΪBuffer�ĳ���, ���ʱΪval��ʵ�ʳ���;
/// @retval kRetOK,  ��ȡ�ɹ�;
///         kRetRecordNotFind, ��¼δ�ҵ�;
///         kRetBufferOverflow, ��¼�ҵ�,���Ƿ�����ڴ�ռ䲻����
///                             val_len�᷵����Ҫ���ڴ泤�ȡ�
RetCode SSTableReader::FindFirstValue(const void* key, uint16_t key_len,
        void* val, uint32_t* val_len)
{
    ReaderBlock* cur_block = NULL;
    uint32_t block_no = kMaxNumOfRecords;
    uint32_t record_no = kMaxNumOfRecords;
    uint32_t basekey_record_no = kMaxNumOfRecords;

    RetCode ret = kRetOK;
    bool is_exsited = this->SeekRecord(key, key_len, &cur_block,
                                       &block_no, &record_no, &basekey_record_no);

    if (is_exsited)
    {
        ret = cur_block->GetValByRecordNo(record_no, val, val_len);
        ClearBlock(block_no);
    }
    else
    {
        ret = kRetRecordNotFind;
    }

    return ret;
}

/// @brief  ͨ��Key���ҵ�һ��value, �Ե���sstable�������ȡ,
///         �ú������ڵ�����û�з����ڴ�ʱʱʹ�á�
/// @param  key, ����key;
/// @param  key_len, ����key len;
/// @param  val, string���ͷ���val;
/// @retval true, �ҵ���Ӧ�ļ�¼;
///         false, û���ҵ���Ӧ�ļ�¼;
bool SSTableReader::FindFirstValue(const void* key, uint16_t key_len,
        std::string* val)
{
    ReaderBlock* cur_block = NULL;
    uint32_t block_no = kMaxNumOfRecords;
    uint32_t record_no = kMaxNumOfRecords;
    uint32_t basekey_record_no = kMaxNumOfRecords;

    bool is_exsited = this->SeekRecord(key, key_len, &cur_block,
                                       &block_no, &record_no, &basekey_record_no);

    if (is_exsited)
    {
        cur_block->GetValByRecordNo(record_no, val);
        ClearBlock(block_no);
    }

    return  is_exsited;
}

/// @brief  ͳ�����п�Ŀ��, ��ʼƫ��, �鳤��
/// @param  block_infos, �������, ������п���Ϣ
/// @retval true, �ɹ�, false, block_infos��������Ч
bool SSTableReader::GetBlockInfos(std::vector<BlockInfo>* block_infos)
{
  if ( block_infos == NULL ) return false;
  block_infos->clear();

  uint32_t  block_no;
  FileReadInfo read_info;
  BlockInfo block_info;

  uint32_t num_blocks = GetNumBlocks();
  for (block_no = 0; block_no < num_blocks; block_no++)
  {
    ComputeFileReadInfo(block_no, false, &read_info);
    block_info.block_no = block_no;
    block_info.start_offset = read_info.start_offset;
    block_info.length_bytes = read_info.read_bytes;
    block_infos->push_back(block_info);
  }

  return true;
}

/// @brief  �������в���һ��key�����Ŀ��, ��¼�ţ���ѹ������key�ļ�¼��;
/// @param  key, key��ָ��;
/// @param  key_len, key�ĳ���;
/// @param  block, �������,block��ָ��;
/// @param  block_no, �������, key�����Ŀ��;
/// @param  record_no, �������, �ڿ��еļ�¼��;
/// @param  basekey_record_no, �������, ��¼��basekey�ļ�¼��;
/// @retval true,�ҵ���false, �Ҳ���
bool SSTableReader::SeekRecord(const void* key, uint16_t key_len,
                               ReaderBlock** block, uint32_t* block_no,
                               uint32_t* record_no, uint32_t* basekey_record_no)
{
    assert((key != NULL) && (key_len != 0));
    assert((block_no != NULL) && (record_no != NULL) && (basekey_record_no != NULL));
    assert(block != NULL);
    assert((m_sstable_header != NULL) && (m_index_block != NULL));
    RetCode ret = kRetOK;
    bool is_accurate_hit = false;

    // 1:����BloomFilter;
    if (m_bloomfilter != NULL && !m_bloomfilter->MayContain(key, key_len))
    {
        *block = NULL;
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    // 2:���ҵ���Ӧ��Block;
    ret = m_index_block->FindRecordNoByKey(key,
                                           key_len,
                                           block_no,
                                           basekey_record_no,
                                           ReaderBlock::kLowBoundFind,
                                           &is_accurate_hit);

    if (ret == kRetRecordNotFind)
    {
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    // 3:װ�ظ�Block;
    *block = LoadDataBlock(*block_no);

    if (*block == NULL)
    {
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    // 4:��Block�в��Ҷ�Ӧ�ļ�¼;
    ret = (*block)->FindRecordNoByKey(key,
                                      key_len,
                                      record_no,
                                      basekey_record_no,
                                      ReaderBlock::kAccurateFind,
                                      &is_accurate_hit);

    if (ret == kRetRecordNotFind)
    {
        // ��Ҫ�����block;
        ClearBlock(*block_no);
        (*block) = NULL;
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    return true;
}

/// @brief  ��������low_bound����һ��key�����Ŀ��, ��¼�ţ���ѹ������key�ļ�¼��;
/// @param  key, key��ָ��;
/// @param  key_len, key�ĳ���;
/// @param  block, �������,block��ָ��;
/// @param  block_no, �������, key�����Ŀ��;
/// @param  record_no, �������, �ڿ��еļ�¼��;
/// @param  basekey_record_no, �������, ��¼��basekey�ļ�¼��;
/// @param  is_accurate_hit, �������, ��ʾ�Ƿ�ȷ����;
/// @retval true,�ҵ���false, �Ҳ���
bool SSTableReader::LowBoundSeekRecord(const void* key, uint16_t key_len,
                                       ReaderBlock** block, uint32_t* block_no,
                                       uint32_t* record_no, uint32_t* basekey_record_no,
                                       bool* is_accurate_hit)
{
    assert((key != NULL) && (key_len != 0));
    assert(block != NULL);
    assert((block_no != NULL) && (record_no != NULL) && (basekey_record_no != NULL));
    assert((m_sstable_header != NULL) && (m_index_block != NULL));
    RetCode ret = kRetOK;

    // 1: ���ҵ���Ӧ��Block;
    ret = m_index_block->FindRecordNoByKey(key, key_len, block_no,
              basekey_record_no, ReaderBlock::kLowBoundFind, is_accurate_hit);

    if (ret == kRetRecordNotFind)
    {
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    // 2: װ�ظ�Block;
    (*block) = LoadDataBlock(*block_no);

    if ((*block) == NULL)
    {
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    // 3: ��Block�в��Ҷ�Ӧ�ļ�¼;
    ret = (*block)->FindRecordNoByKey(key, key_len, record_no,
                        basekey_record_no, ReaderBlock::kLowBoundFind, is_accurate_hit);

    if (ret == kRetRecordNotFind)
    {
        // ��Ҫ�����Block;
        ClearBlock(*block_no);
        (*block) = NULL;
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    return true;
}

/// @brief  ���ݿ��װ��һ��Block;
/// @param  block_no, block id��;
/// @retval ���ݿ��ָ��;
ReaderBlock*  SSTableReader::LoadDataBlock(uint32_t block_no)
{
    assert((m_index_block != NULL) && (block_no < m_index_block->GetNumRecords()));
    assert((m_sstable_header != NULL) && (m_file != NULL));

    RetCode ret = kRetOK;
    MutexLocker locker(&m_mutex);

    if (m_blocks[block_no] == NULL)
    {
        bool cache_fetch_ret = false;
        BlockCache* cache = BlockCache::GetCache();
        char* block_buffer = NULL;
        uint32_t buffer_len = 0;

        if (cache != NULL)
        {
            cache_fetch_ret = cache->FetchBlock(m_sstable_header->sstable_id(),
                                                block_no, &block_buffer, &buffer_len);
        }

        if (cache_fetch_ret)
        {
            // cache �����ҵ��˸��ڴ��;
            m_blocks[block_no] = new ReaderBlock(m_sstable_header);
            m_blocks[block_no]->ParseBlockData(block_buffer, buffer_len);
            m_blocks[block_no]->SetMemoryFromCache();
        }
        else
        {
            // ��Ҫ��File���ж�ȡ���ݿ�, ���Ҷ����ݿ���н�ѹ����;
            FileReadInfo cur_read_info;
            ComputeFileReadInfo(block_no, false, &cur_read_info);
            // 2:���ļ�;
            uint32_t  file_error_code = 0;
            int64_t seek_ret = m_file->Seek(cur_read_info.start_offset,
                                            SEEK_SET, &file_error_code);

            if (file_error_code != ERR_FILE_OK)
            {
                LOG(ERROR) << "load block data seek failed, file name:"
                           << m_file_name.c_str()
                           << "error no:" << file_error_code
                           << "block_offset:" << cur_read_info.start_offset;
                return NULL;
            }

            assert(seek_ret == cur_read_info.start_offset);
            (void) seek_ret;

            // check read buffer size;
            if (cur_read_info.read_bytes > m_file_read_buffer_size)
            {
                delete []m_file_read_buffer;
                m_file_read_buffer = new char[cur_read_info.read_bytes];
                m_file_read_buffer_size = cur_read_info.read_bytes;
            }

            int64_t  file_read_len = m_file->Read(m_file_read_buffer,
                                                  cur_read_info.read_bytes,
                                                  &file_error_code);

            if (file_error_code != ERR_FILE_OK)
            {
                LOG(ERROR) << "load block data read data failed, file name:"
                           << m_file_name.c_str()
                           << "error no:" << file_error_code
                           << "block_offset:" << cur_read_info.start_offset
                           << "block len:" << cur_read_info.read_bytes;
                return NULL;
            }

            assert(file_read_len == cur_read_info.read_bytes);
            (void) file_read_len;

            m_blocks[block_no] = new ReaderBlock(m_sstable_header);
            ret = m_blocks[block_no]->UnCompressBlockData(m_file_read_buffer,
                    cur_read_info.read_bytes);

            if (ret != kRetOK)
            {
                delete m_blocks[block_no];
                m_blocks[block_no] = NULL;
                return NULL;
            }
        }
    }

    assert(m_blocks[block_no] != NULL);
    m_blocks[block_no]->IncRef();
    return m_blocks[block_no];
}

/// @brief  ���ݿ��װ��һ��Block; ��Ԥȡ����
///         ����Ŀռ�<kXFSDefaultReadSize,
///         ��������Ԥ�ڵ���ӽ�kXFSDefaultReadSize������������;
/// @param  block_no, ��ʼblock�Ŀ��;
/// @retval block_no �����ݿ�ָ��;
ReaderBlock*  SSTableReader::LoadBlockWithPrefetch(uint32_t block_no)
{
    assert((m_index_block != NULL) && (block_no < m_index_block->GetNumRecords()));
    assert((m_sstable_header != NULL) && (m_file != NULL));

    RetCode ret = kRetOK;
    MutexLocker locker(&m_mutex);

    if (m_blocks[block_no] == NULL)
    {
        bool cache_fetch_ret = false;
        BlockCache* cache = BlockCache::GetCache();
        char* block_buffer = NULL;
        uint32_t buffer_len = 0;

        if (cache != NULL)
        {
            cache_fetch_ret = cache->FetchBlock(m_sstable_header->sstable_id(),
                                                block_no, &block_buffer, &buffer_len);
        }

        if (cache_fetch_ret)
        {
            // cache �����ҵ��˸��ڴ��;
            m_blocks[block_no] = new ReaderBlock(m_sstable_header);
            m_blocks[block_no]->ParseBlockData(block_buffer, buffer_len);
            m_blocks[block_no]->SetMemoryFromCache();
        }
        else
        {
            // ��Ҫ��File���ж�ȡ���ݿ�, ���Ҷ����ݿ���н�ѹ����;

            // 1:ȡ��ǰBlock�ĳ��Ⱥ�Offset;
            FileReadInfo cur_read_info;
            ComputeFileReadInfo(block_no, true, &cur_read_info);

            // 2:���ļ�;
            uint32_t  file_error_code = 0;
            int64_t seek_ret = m_file->Seek(cur_read_info.start_offset,
                                            SEEK_SET, &file_error_code);

            if (file_error_code != ERR_FILE_OK)
            {
                LOG(ERROR) << "load block data seek failed, file name:"
                           << m_file_name.c_str()
                           << "error no:" << file_error_code
                           << "block_offset:" << cur_read_info.start_offset;
                return NULL;
            }

            assert(seek_ret == cur_read_info.start_offset);
            (void) seek_ret;

            // check read buffer size;
            if (cur_read_info.read_bytes > m_file_read_buffer_size)
            {
                delete []m_file_read_buffer;
                m_file_read_buffer = new char[cur_read_info.read_bytes];
                m_file_read_buffer_size = cur_read_info.read_bytes;
            }

            int64_t  file_read_len = m_file->Read(m_file_read_buffer,
                                                  cur_read_info.read_bytes,
                                                  &file_error_code);

            if (file_error_code != ERR_FILE_OK)
            {
                LOG(ERROR) << "load block data read data failed, file name:"
                           << m_file_name.c_str()
                           << "error no:" << file_error_code
                           << "block_offset:" << cur_read_info.start_offset
                           << "block len:" << cur_read_info.read_bytes
                           << "block num:" << cur_read_info.read_num_blocks;
                return NULL;
            }

            assert(file_read_len == cur_read_info.read_bytes);
            (void)  file_read_len;

            // ��ѹ�ͽ�����;
            uint32_t block_count = 0;
            int64_t cur_offset = 0;
            uint32_t cur_length = 0;

            while (block_count < cur_read_info.read_num_blocks)
            {
                cur_length = (cur_read_info.lens_of_blocks)[block_count];

                m_blocks[block_no + block_count] = new ReaderBlock(m_sstable_header);
                ret = m_blocks[block_no + block_count]->UnCompressBlockData(m_file_read_buffer +
                    cur_offset, cur_length);

                if (ret != kRetOK)
                {
                    delete m_blocks[block_no + block_count];
                    m_blocks[block_no + block_count] = NULL;
                    LOG(ERROR) << "uncompress block failed, file name:"
                               << m_file_name.c_str()
                               << "block no:" << block_no + block_count
                               << "compressed data len: "
                               << (cur_read_info.lens_of_blocks)[block_count];

                    // һ�������ʧ��֮��, ���治�ٴ���;
                    break;
                }

                cur_offset += cur_length;
                block_count++;
            }

            if (m_blocks[block_no] == NULL)
            {
                // ��һ���鶼û�н�ѹ�ɹ�, ֱ�ӷ���NULL;
                return NULL;
            }
        }
    }

    m_blocks[block_no]->IncRef();
    return m_blocks[block_no];
}

/// @brief ���һ��Block��; ��ʼ����Cacheʱ��
///        ��ջὫBlock��Buffer���뵽Cache����;
/// @param block_no, ���;
void  SSTableReader::ClearBlock(uint32_t block_no)
{
    if (block_no == kMaxNumOfRecords) return;

    BlockCache* cache = BlockCache::GetCache();

    if (cache == NULL)   // ��ʹ��cacheʱ;
    {
        ReleaseBlock(block_no);
        return;
    }

    // ��Ҫ������block���ڴ�, ���뵽cache����;
    char*    block_buffer  = NULL;
    uint32_t buffer_len    = 0;
    bool     is_from_cache = false;

    bool b_ret = ReleaseAndSwapOutBlock(block_no,
                                        &block_buffer, &buffer_len, &is_from_cache);

    if (!b_ret) return;

    // ��Ҫɾ��;
    uint64_t sstable_id = m_sstable_header->sstable_id();

    if (is_from_cache)
    {
        // �ڴ�����Cache, ��Ҫ�������ü���;
        cache->ReleaseBlock(sstable_id, block_no);
    }
    else
    {
        // ��Ҫ��Block���뵽Cache����;
        bool push_ret = cache->PushBlock(sstable_id, block_no,
                                         block_buffer, buffer_len);

        if (!push_ret)
        {
            // ����cache ʧ�ܵ����Ѿ�������־, ��Ҫɾ����block_buffer;
            delete [] block_buffer;
            block_buffer = NULL;
        }
    }
}

/// @brief  �ͷ�block�е���Դ, �������ü����������ü���Ϊ0, ��Block��ɾ����
/// @param  block_no, block�ı��;
/// @retval ��,
void  SSTableReader::ReleaseBlock(uint32_t block_no)
{
    assert((m_index_block != NULL) && (block_no < m_index_block->GetNumRecords()));
    MutexLocker locker(&m_mutex);

    if (m_blocks[block_no] == NULL) return;

    uint32_t cur_ref = m_blocks[block_no]->DecRef();

    if (cur_ref == 0)
    {
        delete m_blocks[block_no];
        m_blocks[block_no] = NULL;
    }
}

/// @brief  �ͷ�block�е���Դ, �������ü�����
///         �����ü���Ϊ0, ��Block���е�Buffer���أ�
///         ���������Block��Buffer��ָ��,����ɾ����Block����;
/// @param  block_no, block�ı��;
/// @param  buffer,  ������, Buffer��ָ��;
/// @param  buffer_len, ������, buffer�ĳ���;
/// @param  is_from_cache,  ������, ��ʶ�ڴ��Ƿ�����Cache;
/// @retval false, ��ʾ���ü�����Ϊ0, ���������Block;
///         true, ��ʾ�Ѿ���Ҫɾ����Block;
bool  SSTableReader::ReleaseAndSwapOutBlock(uint32_t block_no, char** buffer,
        uint32_t* buffer_len, bool* is_from_cache)
{
    assert((m_index_block != NULL) && (block_no < m_index_block->GetNumRecords()));
    bool is_deleted = false;
    MutexLocker locker(&m_mutex);

    if (m_blocks[block_no] == NULL) return false;

    uint32_t cur_ref = m_blocks[block_no]->DecRef();

    if (cur_ref == 0)
    {
        (*is_from_cache) = m_blocks[block_no]->IsMemoryFromCache();
        m_blocks[block_no]->SwapOutBuffer(buffer, buffer_len);
        delete m_blocks[block_no];
        m_blocks[block_no] = NULL;
        is_deleted = true;
    }

    return is_deleted;
}

/// @brief   ����block_no��Ԥȡ��־��������Ҫ��ȡ�����������Ŀ,
///          �ļ�����ʼ��ȡ�㣬�Լ���Ҫ��ȡ�ĳ���;
/// @param   block_no, ��ʼ��block��;
/// @param   need_prefetch, �Ƿ���Ҫprefetch;
/// @param   read_info, ��Ҫ���ļ��ж�����Ϣ;
void  SSTableReader::ComputeFileReadInfo(uint32_t block_no,
        bool need_prefetch,
        FileReadInfo* read_info)
{
    assert(read_info != NULL);
    RetCode ret = kRetOK;

    // 1: ȡ��ȡ����ʼoffset;
    char cur_offset_val[8]; // 8���ֽڵ�offset;
    uint32_t cur_offset_val_len = 8;
    uint64_t* offset_ptr = NULL;

    if (block_no == 0)
    {
        read_info->start_offset = 0;
    }
    else
    {
        ret = m_index_block->GetValByRecordNo(block_no - 1,
                                              cur_offset_val, &cur_offset_val_len);
        assert(ret == kRetOK);
        offset_ptr = reinterpret_cast<uint64_t*>(cur_offset_val);
        read_info->start_offset = static_cast<int64_t>(::ntohll(*offset_ptr));
    }

    // 2: ȡ��ȡ�ĳ��Ⱥ���Ҫ��ȡ��block��Ŀ;
    ret = m_index_block->GetValByRecordNo(block_no,
                                          cur_offset_val, &cur_offset_val_len);
    assert(ret == kRetOK);
    offset_ptr = reinterpret_cast<uint64_t*>(cur_offset_val);
    int64_t  cur_offset = static_cast<int64_t>(::ntohll(*offset_ptr));
    (read_info->lens_of_blocks).push_back(cur_offset - read_info->start_offset);
    uint64_t sstable_id = m_sstable_header->sstable_id();
    uint32_t end_block_no = block_no + 1;

    if (!need_prefetch)
    {
        read_info->read_bytes = cur_offset - read_info->start_offset;
        read_info->read_num_blocks = end_block_no - block_no;
        return;
    }

    // һֱԤȡ������ >= kXFSDefaultReadSize�Ŀ�;
    BlockCache* cache = BlockCache::GetCache();
    int64_t last_block_offset = 0;
    uint32_t num_blocks = m_index_block->GetNumRecords();

    if (cache != NULL)   // ��cache
    {
        while (((cur_offset - read_info->start_offset) < kXFSDefaultReadSize)
               && (m_blocks[end_block_no] == NULL)
               && (end_block_no < num_blocks)
               && (!cache->Existed(sstable_id, end_block_no)))
        {
            last_block_offset = cur_offset;
            ret = m_index_block->GetValByRecordNo(end_block_no,
                                                  cur_offset_val, &cur_offset_val_len);
            assert(ret == kRetOK);
            offset_ptr = reinterpret_cast<uint64_t*>(cur_offset_val);
            cur_offset = static_cast<int64_t>(::ntohll(*(offset_ptr)));
            (read_info->lens_of_blocks).push_back(cur_offset - last_block_offset);
            end_block_no++;
        }
    }
    else
    {
        while (((cur_offset - read_info->start_offset) < kXFSDefaultReadSize)
               && (m_blocks[end_block_no] == NULL)
               && (end_block_no < num_blocks))
        {
            last_block_offset = cur_offset;
            ret = m_index_block->GetValByRecordNo(end_block_no,
                                                  cur_offset_val, &cur_offset_val_len);
            assert(ret == kRetOK);
            offset_ptr = reinterpret_cast<uint64_t*>(cur_offset_val);
            cur_offset = static_cast<int64_t>(::ntohll(*(offset_ptr)));
            (read_info->lens_of_blocks).push_back(cur_offset - last_block_offset);
            end_block_no++;
        }
    }

    read_info->read_bytes = cur_offset - read_info->start_offset;
    read_info->read_num_blocks = end_block_no - block_no;
}

/// @brief  װ��schema;
/// @param  ��
/// @retval kRetOK: �ɹ�
///         kRetIOError:  IO��;
RetCode SSTableReader::LoadSSTableHeader()
{
    assert(m_file != NULL);
    RetCode ret = kRetOK;

    MutexLocker locker(&m_mutex);
    uint32_t  file_error_code = 0;

    // ��ȡ�����ֶ�
    int64_t  reading_header_len = 0;
    int64_t  file_len = ::File::GetSize(m_file_name.c_str(), &file_error_code);
    if (file_error_code != ERR_FILE_OK)
    {
        LOG(ERROR) << "tell file failed, file name:"
            << m_file_name.c_str()
            << "error no:"
            << file_error_code;

            return kRetIOError;
    }

    m_file->Seek(file_len - sizeof(uint64_t), SEEK_SET, &file_error_code);
    assert(file_error_code == ERR_FILE_OK);

    int64_t  file_read_len = m_file->Read(reinterpret_cast<char*>(&reading_header_len),
                                          sizeof(uint64_t), &file_error_code);

    if (file_error_code != ERR_FILE_OK)
    {
        LOG(ERROR) << "load header length failed, file name:"
                   << m_file_name.c_str()
                   << "error no:"
                   << file_error_code;
        return kRetIOError;
    }

    assert(file_read_len == sizeof(uint64_t));
    uint64_t header_len = ::ntohll(reading_header_len);

    // ��ȡheader
    m_file->Seek(file_len - (header_len + sizeof(uint64_t)), SEEK_SET, &file_error_code);

    if (file_error_code != ERR_FILE_OK)
    {
        LOG(ERROR) << "back seek failed, error no:" << file_error_code
                   << "seek length:" << header_len + sizeof(uint64_t);
        return kRetIOError;
    }

    if (header_len > m_file_read_buffer_size)
    {
        delete [] m_file_read_buffer;
        m_file_read_buffer_size = header_len;
        m_file_read_buffer = new char[m_file_read_buffer_size];
    }

    file_read_len = m_file->Read(m_file_read_buffer,
                                 header_len,
                                 &file_error_code);

    if (file_error_code != ERR_FILE_OK)
    {
        LOG(ERROR) << "read header failed, header length:"
                   << header_len
                   << "error no:" << file_error_code;
        return kRetIOError;
    }

    assert(m_sstable_header == NULL);
    assert(file_read_len == static_cast<int64_t>(header_len));

    // parse protobuf;
    m_sstable_header = new SSTableHeader();
    bool parse_ret = m_sstable_header->ParseFromArray(m_file_read_buffer, header_len);

    if (!parse_ret)
    {
        delete m_sstable_header;
        m_sstable_header = NULL;
        LOG(ERROR) << "parse header failed, header length:" << header_len;
        ret = kRetIOError;
    }

    return ret;
}

/// @brief  װ��Index;
/// @param  ��
/// @retval kRetOK:     �ɹ�
///         kRetIOError: load indexʧ��
RetCode SSTableReader::LoadIndex()
{
    assert((m_file != NULL) && (m_sstable_header != NULL));
    RetCode ret = kRetOK;

    MutexLocker locker(&m_mutex);
    uint32_t  file_error_code = 0;

    uint64_t index_offset = m_sstable_header->index_offset();
    uint64_t index_length = m_sstable_header->index_compressed_length();
    uint64_t index_raw_length = m_sstable_header->index_raw_length();

    if (index_length > m_file_read_buffer_size)
    {
        delete []m_file_read_buffer;
        m_file_read_buffer = new char[index_length];
        m_file_read_buffer_size = index_length;
    }

    int64_t seek_ret = m_file->Seek(index_offset, SEEK_SET, &file_error_code);

    if (file_error_code != ERR_FILE_OK)
    {
        LOG(ERROR) << "load index seek failed, fail name:"
                   << m_file_name.c_str()
                   << "index offset:"
                   << index_offset
                   << "index length"
                   << index_length;
        delete m_sstable_header;
        m_sstable_header = NULL;
        return kRetIOError;
    }

    assert(seek_ret == static_cast<int64_t>(index_offset));
    (void) seek_ret;

    int64_t file_read_len = m_file->Read(m_file_read_buffer,
                                         index_length,
                                         &file_error_code);

    if (file_error_code != ERR_FILE_OK)
    {
        LOG(ERROR) << "load index read failed, fail name:"
                   << m_file_name.c_str()
                   << "index offset:"
                   << index_offset
                   << "index length:"
                   << index_length;
        delete m_sstable_header;
        m_sstable_header = NULL;
        return kRetIOError;
    }

    assert(file_read_len == static_cast<int64_t>(index_length));
    (void) file_read_len;

    // ��ѹ���ҽ���index_block;
    m_index_describe.mutable_options()->set_block_size(index_raw_length);
    // index blockΪֵΪ�̶����ȵĿ�;
    m_index_describe.mutable_options()->set_fixed_data_len(sizeof(uint64_t));
    m_index_describe.mutable_options()->set_compress_type(
        m_sstable_header->options().compress_type());
    m_index_describe.mutable_options()->set_kv_type(
        (m_sstable_header->options().kv_type() & kKeyTypeMask) |
        (kTypeFixedLen << 4));

    if ((m_sstable_header->options().kv_type() & kKeyTypeMask) == kTypeFixedLen)
    {
        m_index_describe.mutable_options()->set_fixed_key_len(
            m_sstable_header->options().fixed_key_len());
    }

    assert(m_index_block == NULL);
    m_index_block = new ReaderBlock(&m_index_describe);
    ret = m_index_block->UnCompressBlockData(m_file_read_buffer, index_length);

    if (ret != kRetOK)
    {
        delete m_sstable_header;
        m_sstable_header = NULL;
        delete m_index_block;
        m_index_block = NULL;
    }

    return ret;
}

/// @brief  װ��Bloomfilter;
/// @param  ��
/// @retval kRetOK: �ɹ�
///         kRetIOError:  load bloom filterʧ��
RetCode SSTableReader::LoadBloomfilter()
{
    assert((m_file != NULL) && (m_sstable_header != NULL));
    RetCode ret = kRetOK;

    MutexLocker locker(&m_mutex);
    uint32_t  file_error_code = 0;

    assert(m_bloomfilter == NULL);

    int64_t  bloomfilter_offset =
        m_sstable_header->bloomfilter_block().bloomfilter_offset();
    int64_t  bloomfilter_len =
        m_sstable_header->bloomfilter_block().bloomfilter_length();

    if (bloomfilter_len > m_file_read_buffer_size)
    {
        delete[]m_file_read_buffer;
        m_file_read_buffer = new char[bloomfilter_len];
        m_file_read_buffer_size = bloomfilter_len;
    }

    int64_t seek_ret = m_file->Seek(bloomfilter_offset, SEEK_SET, &file_error_code);

    if (file_error_code != ERR_FILE_OK)
    {
        LOG(ERROR) << "load bloomfilter seek failed, fail name:"
                   << m_file_name.c_str()
                   << "bloomfilter offset:"
                   << bloomfilter_offset
                   << "bloomfilter length:"
                   << bloomfilter_len;

        // ��Ҫɾ��header �� index;
        delete m_sstable_header;
        m_sstable_header = NULL;
        delete m_index_block;
        m_index_block = NULL;

        return kRetIOError;
    }

    assert(seek_ret == bloomfilter_offset);
    (void) seek_ret;

    int64_t file_read_len = m_file->Read(m_file_read_buffer,
                                         bloomfilter_len,
                                         &file_error_code);

    if (file_error_code != ERR_FILE_OK)
    {
        LOG(ERROR) << "load bloomfilter read failed, fail name:"
                   << m_file_name.c_str()
                   << "bloomfilter offset:"
                   << bloomfilter_offset
                   << "bloomfilter length:"
                   << bloomfilter_len;
        // ��Ҫɾ��header �� index;
        delete m_sstable_header;
        m_sstable_header = NULL;
        delete m_index_block;
        m_index_block = NULL;

        return kRetIOError;
    }

    assert(file_read_len == bloomfilter_len);
    (void) file_read_len;

    m_bloomfilter = new BloomFilter(
        m_file_read_buffer,
        static_cast<size_t>(bloomfilter_len),
        m_sstable_header->bloomfilter_block().bloomfilter_hash_num());
    return ret;
}


} // namespace sstable
