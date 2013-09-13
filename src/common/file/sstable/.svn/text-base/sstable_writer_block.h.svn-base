#ifndef SSTABLE_SSTABLE_WRITER_BLOCK_H_
#define SSTABLE_SSTABLE_WRITER_BLOCK_H_

#include <assert.h>
#include <string>
#include "common/base/stdint.h"
#include "common/file/sstable/sstable_block.h"
#include "thirdparty/gtest/gtest.h"

namespace sstable
{
class WriterBlock: public Block
{
public:
    friend class SSTableWriter;

    // friend for test
    friend class WriterBlockTest;
    FRIEND_TEST(WriterBlockTest, ShouldBeBase);
    FRIEND_TEST(WriterBlockTest, ChangeBlockSize);
    FRIEND_TEST(WriterBlockTest, Reset);
    FRIEND_TEST(WriterBlockTest, WriteFixedKeyLenRecord);
    FRIEND_TEST(WriterBlockTest, WriteVarKeyLenRecord);

    // 构造和析构
    explicit WriterBlock(const SSTableHeader* header);
    virtual ~WriterBlock();

    /// accessor and mutator;
    inline int64_t GetWrittenBytes() const
    {
        return m_written_bytes;
    }

    // get uncompressed length
    inline uint32_t GetUncompressedLength() const
    {
        return m_block_header->raw_len();
    }

    inline const std::string& GetLastKey() const
    {
        return m_last_key;
    }

    ///  @brief   压缩Block数据; 并且保存块写入的最后一个记录;
    ///  @param   buffer, 存放压缩后数据的buffer位置;
    ///  @param   buffer_len, 输入为buffer的长度;  输出为压缩后的长度;
    ///  @retval  kRetOK 成功;
    ///           kRetCompressError 压缩失败;
    RetCode CompressBlockData(char* buffer, uint32_t* buffer_len);

    ///  @brief   向Block当中写入一条数据;
    ///  @param   key, 写入记录的key
    ///  @param   key_len, 写入记录的key len
    ///  @param   data, 写入记录的data
    ///  @param   data_len, 写入记录的data len
    ///  @retval  kRetOK: 写入成功
    ///           kRetBlockOverflow: Block已满
    RetCode WriteRecord(const void* key,
                        uint16_t key_len,
                        const void* data,
                        uint32_t data_len);
protected:

    ///  @brief  改变块的大小;
    ///  @param  block_size
    ///  @retval 无
    inline void ChangeBlockSize();

    ///  @brief  重置写入块;
    ///  @params 无
    ///  @retval 无
    inline void Reset();

private:

    /// 预留给sstable block header的长度, 防止采用压缩算法NONE时，分配内存不够的情况
    static const uint32_t kMaxBlockHeaderLen = 64;

    ///  @breif  判断一个待写入的key是否应该独立成basekey;
    ///  @param  key 需要写入的key;
    ///  @param  key_len key的长度;
    ///  @param  prefix_len, 若不需要为base时,返回和当前base的prefix_len
    ///  @retval true, 需要为base;
    ///          fasle,不需要为base;
    bool ShouldBeBase(const void* key,
                      uint16_t key_len,
                      uint16_t* prefix_len) const;

    ///  @brief   固定长度key的记录的写入;
    ///  @param   key, 写入记录的key
    ///  @param   key_len, 写入记录的key len
    ///  @param   data, 写入记录的data
    ///  @param   data_len, 写入记录的data len
    ///  @retval  kRetOK: 写入成功
    ///           kRetBlockOverflow: Block已满,没写成功;
    RetCode WriteFixedKeyLenRecord(const void* key,
                                   uint16_t key_len,
                                   const void* val,
                                   uint32_t val_len);

    ///  @brief   固定长度key的记录的写入;
    ///  @param   key, 写入记录的key
    ///  @param   key_len, 写入记录的key len
    ///  @param   data, 写入记录的data
    ///  @param   data_len, 写入记录的data len
    ///  @retval  kRetOK: 写入成功
    ///           kRetBlockOverflow: Block已满,没写成功;
    RetCode WriteVarKeyLenRecord(const void* key,
                                 uint16_t key_len,
                                 const void* val,
                                 uint32_t val_len);


    ///  @brief   压缩Block头数据;
    ///  @param   buffer, 存放压缩后数据的buffer位置;
    ///  @param   buffer_len, 输入为buffer的长度;  输出为压缩后的长度;
    ///  @param   is_writter, true表示要写入到buffer;  false表示只计算长度，不写入buffer;
    ///  @retval  kRetOK 成功;
    ///           kRetCompressError 压缩失败;
    RetCode CompressBlockHeader(char* buffer, uint32_t* buffer_len, bool is_writer = true);


    ///  @brief   压缩offset数据; 将offset数据存放为Variant Integer;
    ///  @param   buffer, 存放压缩后offset数据的buffer位置;
    ///  @param   buffer_len, buffer的长度;
    void CompressOffset(char* buffer, uint32_t* buffer_len, uint32_t* raw_offsets_len);


    int64_t        m_written_bytes;    ///< 已经写了的字节数;

    char*           m_prev_basekey_ptr; ///< 上一条写入key的basekey的ptr;
    uint16_t        m_prev_basekey_len; ///< 上一条写入key的basekey的长度;
    char*           m_prev_key_ptr;     ///< 上一条写入key的ptr;
    uint16_t        m_prev_key_len;     ///< 上一条写入key的长度;
    uint16_t        m_prev_prefix_len;  ///< 上一条写入key与basekey相同的前缀串长度;

    char*           m_key_next_write_ptr;           ///< 下一个key写入的地址指针;
    uint32_t*       m_key_offset_next_write_ptr;    ///< 下一个keyOffset写入的地址指针;
    uint32_t*       m_basekey_index_next_write_ptr; ///< 下一个basekey的recordNo写入地址指针;
    uint32_t*       m_val_offset_next_write_ptr;    ///< 下一个值的Offset写入的地址指针;
    char*           m_val_next_write_ptr;           ///< 下一个val的写入地址指针;

    std::string     m_last_key;                     ///< 最后一个key;
}; // class

///  @brief  改变块的大小;
///  @param  block_size
///  @retval 无
inline void WriterBlock::ChangeBlockSize()
{
    char* old_key_start_ptr = m_key_start_ptr;
    char* old_val_start_ptr = m_val_ptr;

    uint32_t written_key_len = static_cast<uint32_t>
                               (m_key_next_write_ptr - m_key_start_ptr);

    uint32_t cur_block_size = m_sstable_header->options().block_size();
    m_key_start_ptr = new char[cur_block_size];
    ::memcpy(m_key_start_ptr, old_key_start_ptr, written_key_len);
    m_key_next_write_ptr = m_key_start_ptr + written_key_len;

    if ((m_sstable_header->options().kv_type() & kKeyTypeMask) == kTypeFixedLen)
    {
        /// 固定长度key的块;
        m_key_common_prefix = m_key_start_ptr;
    }

    uint32_t written_val_len = static_cast<uint32_t>
                               (m_val_next_write_ptr - m_val_ptr);
    m_val_ptr = new char[cur_block_size];
    ::memcpy(m_val_ptr, old_val_start_ptr, written_val_len);
    m_val_next_write_ptr = m_val_ptr + written_val_len;

    int64_t old_offset = m_key_start_ptr - old_key_start_ptr;
    m_prev_basekey_ptr += old_offset;
    m_prev_key_ptr += old_offset;

    delete []old_key_start_ptr;
    delete []old_val_start_ptr;
}

///  @brief  重置写入块;
///  @params 无
///  @retval 无
inline void WriterBlock::Reset()
{
    assert(m_block_header != NULL);
    m_block_header->Clear();

    m_written_bytes = kMaxBlockHeaderLen;
    m_prev_basekey_ptr = NULL;
    m_prev_basekey_len = 0;
    m_prev_key_ptr = NULL;
    m_prev_key_len = 0;
    m_prev_prefix_len = 0;

    m_key_next_write_ptr = m_key_start_ptr;

    if ((m_sstable_header->options().kv_type() & kKeyTypeMask) == kTypeFixedLen)
    {
        /// 固定长度key的块;
        m_key_common_prefix = m_key_start_ptr;
    }
    else
    {
        /// 变长度key的块;
        m_key_offset_next_write_ptr = m_key_offset_ptr;
        m_basekey_index_next_write_ptr = m_basekey_index_ptr;
    }

    m_val_next_write_ptr = m_val_ptr;

    if (((m_sstable_header->options().kv_type() & kValTypeMask) >> 4) ==
        kTypeVariableLen)
    {
        /// 变长val块;
        m_val_offset_next_write_ptr = m_val_offset_ptr;
    }
}

} // namespace sstable

#endif // SSTABLE_SSTABLE_WRITER_BLOCK_H_
