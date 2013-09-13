#ifndef SSTABLE_SSTABLE_READER_H_
#define SSTABLE_SSTABLE_READER_H_

#include <string>
#include <vector>
#include "common/base/stdint.h"
#include "common/collection/bloom_filter.hpp"
#include "common/system/concurrency/mutex.hpp"
#include "common/file/sstable/sstable_def.h"
#include "common/file/sstable/sstable_header.pb.h"
#include "common/file/sstable/sstable_block.h"
#include "common/file/sstable/sstable_reader_block.h"
#include "thirdparty/gtest/gtest.h"

class File;
namespace sstable
{
class SSTableReaderIterator;

class SSTableReader
{
public:
    friend class SSTableReaderIterator;

    // friend for test
    friend class ReaderTest;
    FRIEND_TEST(ReaderTest, LoadDataBlock);
    FRIEND_TEST(ReaderTest, LoadBlockWithPrefetch);
    FRIEND_TEST(ReaderTest, FindFirstValue);
    FRIEND_TEST(ReaderTest, SeekRecord);
    FRIEND_TEST(ReaderTest, LowBoundSeekRecord);

    typedef  SSTableReaderIterator  Iterator;

    // 构造和析构
    explicit SSTableReader(const char* file_path):
        m_file(NULL),
        m_file_read_buffer(NULL),
        m_file_read_buffer_size(kXFSDefaultReadSize),
        m_sstable_header(NULL),
        m_index_block(NULL),
        m_bloomfilter(NULL),
        m_blocks(NULL),
        m_memmap_called(false),
        m_iterator_count(0),
        m_file_name(file_path)
    {
        m_file_read_buffer = new char[kXFSDefaultReadSize];
        m_file_read_buffer_size = kXFSDefaultReadSize;
    }

    ~SSTableReader();

    // accesser and mutator
    const std::string&  GetFileName() const
    {
        return m_file_name;
    }

    const SSTableHeader*  GetHeader() const
    {
        return m_sstable_header;
    }

    /// @brief  同步打开一个sstable 文件, 会读入schema, block index, bloomfilter;
    /// @param  file_path, 文件的路径;
    /// @retval kRetOK:      成功
    ///         kRetIOError:  打开文件失败
    RetCode OpenFile();

    /// @brief  同步关闭一个sstable文件;
    /// @param  文件对象的指针;
    /// @retval kRetOK:       成功;
    ///         kRetIOError:  关闭文件失败;
    RetCode CloseFile();

    /// @brief  同步将一个sstable文件的数据map到内存当中;
    /// @param  无
    /// @retval kRetOK:  正确，
    ///         <0: 出错，错误码;
    RetCode MemMap();

    /// @brief  创建一个iterator, 不做定位操作;
    /// @retval iterator的指针;
    Iterator* NewIterator();

    /// @brief  创建一个iterator, 并且将iterator的位置定位到第一块记录;
    /// @retval iterator的指针;
    Iterator* CreateIterator();

    /// @brief  通过Key查找第一个值, 对多个sstable的随机读取,
    ///         该函数用于调用者已经分配存储val的内存时使用。
    /// @param  key, 输入key;
    /// @param  key_len, 输入key len;
    /// @param  val, 用于存储first_value的buffer的指针;
    /// @param  val_len, 输入时为Buffer的长度, 输出时为val的实际长度;
    /// @retval kRetOK,  读取成功;
    ///         kRetRecordNotFind, 记录未找到;
    ///         kRetBufferOverflow, 记录找到,但是分配的内存空间不够，
    ///                             val_len会返回需要的内存长度。
    RetCode FindFirstValue(const void* key, uint16_t key_len,
                           void* val, uint32_t* val_len);

    /// @brief  通过Key查找第一个value, 对多个sstable的随机读取,
    ///         该函数用于调用者没有分配内存时时使用。
    /// @param  key, 输入key;
    /// @param  key_len, 输入key len;
    /// @param  val, string类型返回val;
    /// @retval true, 找到对应的记录;
    ///         false, 没有找到对应的记录;
    bool FindFirstValue(const void* key, uint16_t key_len, std::string* val);

    /// @brief  统计所有块的块号, 起始偏移, 块长度
    /// @param  block_infos, 输出参数, 存放所有块信息
    /// @retval true, 成功, false, block_infos中内容无效
    bool GetBlockInfos(std::vector<BlockInfo>* blockinfos);

protected:

    // accesser and mutator
    uint32_t GetNumBlocks() const
    {
        assert(m_index_block != NULL);
        return m_index_block->GetNumRecords();
    }

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

    /// @brief 用于检测是否有Iterator没有释放掉;
    /// @param block_no, block号;
    /// @retval true, 存在leak, false, 不存在leak;
    bool CheckBlockRef(uint32_t block_no)
    {
        MutexLocker locker(&m_mutex);
        bool b_ret = true;

        if ((m_memmap_called)
            && (m_blocks[block_no] != NULL)
            && (m_blocks[block_no]->m_ref == 1))
        {
            b_ret = false;
            return b_ret;
        }

        if ((!m_memmap_called) && (m_blocks[block_no] == NULL))
        {
            b_ret = false;
            return b_ret;
        }

        if ((!m_memmap_called)
            && (m_blocks[block_no] != NULL)
            && (m_blocks[block_no]->m_ref == 0))
        {
            b_ret = false;
        }

        return b_ret;
    }

    /// @brief 清空一个Block块; 初始化了Cache时，
    ///        清空会将Block的Buffer放入到Cache当中;
    /// @param block_no, 块号;
    void  ClearBlock(uint32_t block_no);

    /// @brief  在索引中查找一个key所属的块号, 记录号，和压缩基础key的记录号;
    /// @param  key, key的指针;
    /// @param  key_len, key的长度;
    /// @param  block, 输出参数,block的指针;
    /// @param  block_no, 输出参数, key所属的块号;
    /// @param  record_no, 输出参数, 在块中的记录号;
    /// @param  basekey_record_no, 输出参数, 记录的basekey的记录号;
    /// @retval true,找到，false, 找不到
    bool SeekRecord(const void* key, uint16_t key_len,
                    ReaderBlock** block, uint32_t* block_no,
                    uint32_t* record_no, uint32_t* basekey_record_no);

    /// @brief  在索引中low_bound查找一个key所属的块号, 记录号，和压缩基础key的记录号;
    /// @param  key, key的指针;
    /// @param  key_len, key的长度;
    /// @param  block, 输出参数,block的指针;
    /// @param  block_no, 输出参数, key所属的块号;
    /// @param  record_no, 输出参数, 在块中的记录号;
    /// @param  basekey_record_no, 输出参数, 记录的basekey的记录号;
    /// @param  is_accurate_hit, 输出参数, 表示是否精确命中;
    /// @retval true,找到，false, 找不到
    bool LowBoundSeekRecord(const void* key, uint16_t key_len,
                            ReaderBlock** block, uint32_t* block_no,
                            uint32_t* record_no, uint32_t* basekey_record_no,
                            bool* is_accurate_hit);

    /// @brief  根据块号装载一个Block;
    /// @param  block_no, block 块号;
    /// @retval 数据块的指针;
    ReaderBlock*  LoadDataBlock(uint32_t block_no);

    /// @brief  根据块号装载一个Block; 带预取机制
    ///         当块的空间<kXFSDefaultReadSize,
    ///         尝试向下预期到最接近kXFSDefaultReadSize的连续几个块; iterator时使用;
    /// @param  block_no, 起始block的块号;
    /// @retval block_no 的数据块指针;
    ReaderBlock*  LoadBlockWithPrefetch(uint32_t block_no);

    /// @brief  增加block 的ref;
    /// @param  block_no 块的块号;
    void IncreaseBlockRef(uint32_t block_no)
    {
        assert((m_blocks != NULL) && (m_blocks[block_no] != NULL));
        MutexLocker locker(&m_mutex);
        m_blocks[block_no]->IncRef();
    }

    /// @brief  释放block中的资源, 减少引用计数，若引用计数为0, 则将Block块删除。
    /// @param  block_no, block的编号;
    /// @retval 无,
    void  ReleaseBlock(uint32_t block_no);

    /// @brief  释放block中的资源, 减少引用计数，
    ///         若引用计数为0, 则将Block块中的Buffer返回，
    ///         并且清除掉Block中Buffer的指针,并且删除该Block对象;
    /// @param  block_no, block的编号;
    /// @param  buffer,  出参数, Buffer的指针;
    /// @param  buffer_len, 出参数, buffer的长度;
    /// @param  is_from_cache,  出参数, 标识内存是否来自Cache;
    /// @retval false, 表示引用计数不为0, 不能清除该Block;
    ///         true, 表示已经需要删除该Block;
    bool  ReleaseAndSwapOutBlock(uint32_t block_no, char** buffer,
                                 uint32_t* buffer_len, bool* is_from_cache);

private:

    static const  uint32_t kXFSDefaultReadSize = 256 * 1024; /// 缺省的XFS每次读的大小;

    struct FileReadInfo
    {
        FileReadInfo():
            read_num_blocks(0),
            start_offset(0),
            read_bytes(0)
        {}
        uint32_t  read_num_blocks;  // 需要读的块数目;
        int64_t   start_offset;     // 文件读的起始Offset;
        int64_t   read_bytes;       // 需要读取的字节数;
        std::vector<uint32_t> lens_of_blocks; // 每个块的长度;
    };

    /// @brief   根据block_no和预取标志来计算需要读取的连续块的数目,
    ///          文件的起始读取点，以及需要读取的长度;
    /// @param   block_no, 起始的block号;
    /// @param   need_prefetch, 是否需要prefetch;
    /// @param   read_info, 需要从文件中读的信息;
    void  ComputeFileReadInfo(uint32_t block_no,
                              bool need_prefetch,
                              FileReadInfo* read_info);

    /// @brief   装载sstable头;
    /// @param   无
    /// @retval  kRetOK: 成功
    ///          kRetIOError: load schema失败
    RetCode     LoadSSTableHeader();

    /// @brief   装载Index;
    /// @params  无
    /// @retval  kRetOK:     成功
    ///          kRetIOError: load index失败
    RetCode     LoadIndex();

    /// @brief   装载Bloomfilter;
    /// @params  无
    /// @retval  kRetOK: 成功
    ///          kRetIOError:  load bloom filter失败
    RetCode     LoadBloomfilter();

    File*        m_file;                  ///< xfs库的文件对象;

    char*        m_file_read_buffer;      /// 文件读的Buffer;
    uint32_t     m_file_read_buffer_size; /// 文件读的Buffer;

    SSTableHeader* m_sstable_header;      ///< sstable的Schema;
    SSTableHeader  m_index_describe;      ///< sstable的index 块的描述;
    ///<   Index的Schema与sstable data的描述不一样，
    ///<   固定为定长值类型的描述;
    ReaderBlock*  m_index_block;          ///< sstable的IndexBlock;
    BloomFilter*  m_bloomfilter;          ///< 指向bloomfilter的指针;
    ReaderBlock** m_blocks;               ///< 存放block的Set;
    bool          m_memmap_called;        ///< 标识是否进行了memmap的调用;
    uint32_t      m_iterator_count;       ///< 从该Reader上创建的iterator的计数器;

    RecursiveMutex  m_mutex;              ///< 线程锁

    std::string  m_file_name;             ///< 文件名;
};
}

#endif // SSTABLE_SSTABLE_READER_H_
