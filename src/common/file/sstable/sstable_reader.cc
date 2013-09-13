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
    // 检测是否所有已经创建的Iterator都已经被删除掉;
    assert(GetIteratorCount() == 0);

    if (m_blocks != NULL)
    {
        assert(m_index_block != NULL);
        uint32_t num_blocks = m_index_block->GetNumRecords();

        for (uint32_t block_count = 0; block_count < num_blocks; block_count++)
        {
            /// 在Debug模式下调用, Release模式不会调用;
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

/// @brief  同步打开一个sstable 文件,
///         会读入schema, block index, bloomfilter;
/// @param  file_path, 文件的路径;
/// @retval kRetOK:      成功
///         kRetIOError:  打开文件失败
RetCode SSTableReader::OpenFile()
{
    RetCode ret = kRetOK;

    VLOG(0) << "open file, file name:" << m_file_name.c_str();

    // 1:打开文件;
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

    // 2:加载Schema;
    ret = LoadSSTableHeader();

    if (ret != kRetOK)
    {
        LOG(ERROR) << "load schema failed, file name:"
                   << m_file_name.c_str();
        return ret;
    }

    // 3:加载Index;
    ret = LoadIndex();

    if (ret != kRetOK)
    {
        LOG(ERROR) << "load index failed, file name:"
                   << m_file_name.c_str();
        return ret;
    }

    // 4:加载Bloomfilter
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

    // 6: 初始化m_blocks;
    m_blocks = new ReaderBlock*[num_blocks];
    for (uint32_t i = 0; i < num_blocks; ++i)
    {
        m_blocks[i] = NULL;
    }

    return ret;
}

/// @brief  同步关闭一个sstable文件;
/// @param  文件对象的指针;
/// @retval kRetOK:       成功;
///         kRetIOError:  关闭文件失败;
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

/// @brief  同步将一个sstable文件的数据map到内存当中;
/// @param  无
/// @retval kRetOK:  正确，
///         kRetIOError: IO错误;
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

/// @brief  创建一个iterator, 不做定位操作;
/// @retval iterator的指针;
SSTableReader::Iterator* SSTableReader::NewIterator()
{
    IncreaseIteratorCount();
    return new Iterator(this);
}

/// @brief  创建一个iterator, 并且将iterator的位置定位到第一块记录;
/// @retval iterator的指针;
SSTableReader::Iterator* SSTableReader::CreateIterator()
{
    Iterator* ret_iter = new Iterator(this);
    ret_iter->Reset();
    IncreaseIteratorCount();
    return ret_iter;
}

/// @brief  通过Key查找第一个值, 对单个sstable的随机读取,
///         该函数用于调用者已经分配存储val的内存时使用。
/// @param  key, 输入key;
/// @param  key_len, 输入key len;
/// @param  val, 用于存储first_value的buffer的指针;
/// @param  val_len, 输入时为Buffer的长度, 输出时为val的实际长度;
/// @retval kRetOK,  读取成功;
///         kRetRecordNotFind, 记录未找到;
///         kRetBufferOverflow, 记录找到,但是分配的内存空间不够，
///                             val_len会返回需要的内存长度。
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

/// @brief  通过Key查找第一个value, 对单个sstable的随机读取,
///         该函数用于调用者没有分配内存时时使用。
/// @param  key, 输入key;
/// @param  key_len, 输入key len;
/// @param  val, string类型返回val;
/// @retval true, 找到对应的记录;
///         false, 没有找到对应的记录;
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

/// @brief  统计所有块的块号, 起始偏移, 块长度
/// @param  block_infos, 输出参数, 存放所有块信息
/// @retval true, 成功, false, block_infos中内容无效
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

/// @brief  在索引中查找一个key所属的块号, 记录号，和压缩基础key的记录号;
/// @param  key, key的指针;
/// @param  key_len, key的长度;
/// @param  block, 输出参数,block的指针;
/// @param  block_no, 输出参数, key所属的块号;
/// @param  record_no, 输出参数, 在块中的记录号;
/// @param  basekey_record_no, 输出参数, 记录的basekey的记录号;
/// @retval true,找到，false, 找不到
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

    // 1:查找BloomFilter;
    if (m_bloomfilter != NULL && !m_bloomfilter->MayContain(key, key_len))
    {
        *block = NULL;
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    // 2:查找到对应的Block;
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

    // 3:装载该Block;
    *block = LoadDataBlock(*block_no);

    if (*block == NULL)
    {
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    // 4:在Block中查找对应的记录;
    ret = (*block)->FindRecordNoByKey(key,
                                      key_len,
                                      record_no,
                                      basekey_record_no,
                                      ReaderBlock::kAccurateFind,
                                      &is_accurate_hit);

    if (ret == kRetRecordNotFind)
    {
        // 需要清除掉block;
        ClearBlock(*block_no);
        (*block) = NULL;
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    return true;
}

/// @brief  在索引中low_bound查找一个key所属的块号, 记录号，和压缩基础key的记录号;
/// @param  key, key的指针;
/// @param  key_len, key的长度;
/// @param  block, 输出参数,block的指针;
/// @param  block_no, 输出参数, key所属的块号;
/// @param  record_no, 输出参数, 在块中的记录号;
/// @param  basekey_record_no, 输出参数, 记录的basekey的记录号;
/// @param  is_accurate_hit, 输出参数, 表示是否精确命中;
/// @retval true,找到，false, 找不到
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

    // 1: 查找到对应的Block;
    ret = m_index_block->FindRecordNoByKey(key, key_len, block_no,
              basekey_record_no, ReaderBlock::kLowBoundFind, is_accurate_hit);

    if (ret == kRetRecordNotFind)
    {
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    // 2: 装载该Block;
    (*block) = LoadDataBlock(*block_no);

    if ((*block) == NULL)
    {
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    // 3: 在Block中查找对应的记录;
    ret = (*block)->FindRecordNoByKey(key, key_len, record_no,
                        basekey_record_no, ReaderBlock::kLowBoundFind, is_accurate_hit);

    if (ret == kRetRecordNotFind)
    {
        // 需要清除掉Block;
        ClearBlock(*block_no);
        (*block) = NULL;
        *block_no = kMaxNumOfRecords;
        *record_no = kMaxNumOfRecords;
        *basekey_record_no = kMaxNumOfRecords;
        return false;
    }

    return true;
}

/// @brief  根据块号装载一个Block;
/// @param  block_no, block id号;
/// @retval 数据块的指针;
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
            // cache 当中找到了该内存块;
            m_blocks[block_no] = new ReaderBlock(m_sstable_header);
            m_blocks[block_no]->ParseBlockData(block_buffer, buffer_len);
            m_blocks[block_no]->SetMemoryFromCache();
        }
        else
        {
            // 需要从File当中读取数据块, 并且对数据块进行解压操作;
            FileReadInfo cur_read_info;
            ComputeFileReadInfo(block_no, false, &cur_read_info);
            // 2:读文件;
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

/// @brief  根据块号装载一个Block; 带预取机制
///         当块的空间<kXFSDefaultReadSize,
///         尝试向下预期到最接近kXFSDefaultReadSize的连续几个块;
/// @param  block_no, 起始block的块号;
/// @retval block_no 的数据块指针;
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
            // cache 当中找到了该内存块;
            m_blocks[block_no] = new ReaderBlock(m_sstable_header);
            m_blocks[block_no]->ParseBlockData(block_buffer, buffer_len);
            m_blocks[block_no]->SetMemoryFromCache();
        }
        else
        {
            // 需要从File当中读取数据块, 并且对数据块进行解压操作;

            // 1:取当前Block的长度和Offset;
            FileReadInfo cur_read_info;
            ComputeFileReadInfo(block_no, true, &cur_read_info);

            // 2:读文件;
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

            // 解压和解析块;
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

                    // 一个块解析失败之后, 后面不再处理;
                    break;
                }

                cur_offset += cur_length;
                block_count++;
            }

            if (m_blocks[block_no] == NULL)
            {
                // 第一个块都没有解压成功, 直接返回NULL;
                return NULL;
            }
        }
    }

    m_blocks[block_no]->IncRef();
    return m_blocks[block_no];
}

/// @brief 清空一个Block块; 初始化了Cache时，
///        清空会将Block的Buffer放入到Cache当中;
/// @param block_no, 块号;
void  SSTableReader::ClearBlock(uint32_t block_no)
{
    if (block_no == kMaxNumOfRecords) return;

    BlockCache* cache = BlockCache::GetCache();

    if (cache == NULL)   // 不使用cache时;
    {
        ReleaseBlock(block_no);
        return;
    }

    // 需要交换出block的内存, 放入到cache当中;
    char*    block_buffer  = NULL;
    uint32_t buffer_len    = 0;
    bool     is_from_cache = false;

    bool b_ret = ReleaseAndSwapOutBlock(block_no,
                                        &block_buffer, &buffer_len, &is_from_cache);

    if (!b_ret) return;

    // 需要删除;
    uint64_t sstable_id = m_sstable_header->sstable_id();

    if (is_from_cache)
    {
        // 内存来自Cache, 需要减少引用计数;
        cache->ReleaseBlock(sstable_id, block_no);
    }
    else
    {
        // 需要将Block插入到Cache当中;
        bool push_ret = cache->PushBlock(sstable_id, block_no,
                                         block_buffer, buffer_len);

        if (!push_ret)
        {
            // 放入cache 失败当中已经加入日志, 需要删除掉block_buffer;
            delete [] block_buffer;
            block_buffer = NULL;
        }
    }
}

/// @brief  释放block中的资源, 减少引用计数，若引用计数为0, 则将Block块删除。
/// @param  block_no, block的编号;
/// @retval 无,
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

/// @brief  释放block中的资源, 减少引用计数，
///         若引用计数为0, 则将Block块中的Buffer返回，
///         并且清除掉Block中Buffer的指针,并且删除该Block对象;
/// @param  block_no, block的编号;
/// @param  buffer,  出参数, Buffer的指针;
/// @param  buffer_len, 出参数, buffer的长度;
/// @param  is_from_cache,  出参数, 标识内存是否来自Cache;
/// @retval false, 表示引用计数不为0, 不能清除该Block;
///         true, 表示已经需要删除该Block;
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

/// @brief   根据block_no和预取标志来计算需要读取的连续块的数目,
///          文件的起始读取点，以及需要读取的长度;
/// @param   block_no, 起始的block号;
/// @param   need_prefetch, 是否需要prefetch;
/// @param   read_info, 需要从文件中读的信息;
void  SSTableReader::ComputeFileReadInfo(uint32_t block_no,
        bool need_prefetch,
        FileReadInfo* read_info)
{
    assert(read_info != NULL);
    RetCode ret = kRetOK;

    // 1: 取读取的起始offset;
    char cur_offset_val[8]; // 8个字节的offset;
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

    // 2: 取读取的长度和需要读取的block数目;
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

    // 一直预取到长度 >= kXFSDefaultReadSize的块;
    BlockCache* cache = BlockCache::GetCache();
    int64_t last_block_offset = 0;
    uint32_t num_blocks = m_index_block->GetNumRecords();

    if (cache != NULL)   // 有cache
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

/// @brief  装载schema;
/// @param  无
/// @retval kRetOK: 成功
///         kRetIOError:  IO错;
RetCode SSTableReader::LoadSSTableHeader()
{
    assert(m_file != NULL);
    RetCode ret = kRetOK;

    MutexLocker locker(&m_mutex);
    uint32_t  file_error_code = 0;

    // 读取长度字段
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

    // 读取header
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

/// @brief  装载Index;
/// @param  无
/// @retval kRetOK:     成功
///         kRetIOError: load index失败
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

    // 解压并且解析index_block;
    m_index_describe.mutable_options()->set_block_size(index_raw_length);
    // index block为值为固定长度的块;
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

/// @brief  装载Bloomfilter;
/// @param  无
/// @retval kRetOK: 成功
///         kRetIOError:  load bloom filter失败
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

        // 需要删除header 和 index;
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
        // 需要删除header 和 index;
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
