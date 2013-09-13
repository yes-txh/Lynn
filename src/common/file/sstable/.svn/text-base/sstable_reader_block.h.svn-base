#ifndef SSTABLE_SSTABLE_READER_BLOCK_H_
#define SSTABLE_SSTABLE_READER_BLOCK_H_

#include <assert.h>
#include <string>
#include "common/base/stdint.h"
#include "common/system/concurrency/mutex.hpp"
#include "common/base/string/string_piece.hpp"
#include "common/encoding/variant_integer.hpp"
#include "common/file/sstable/sstable_block.h"
#include "thirdparty/gtest/gtest.h"

namespace sstable
{
class SSTableReader;
class SSTableReaderIterator;
class ReaderBlock: public Block
{
public:

    friend class SSTableReader;

    friend class ReaderBlockTest;
    FRIEND_TEST(ReaderBlockTest, UnCompressBlockData);

    // 构造和析构
    explicit ReaderBlock(const SSTableHeader* header);
    virtual ~ReaderBlock();

    enum
    {
        kAccurateFind = 0,   /// <精确查找,
        kLowBoundFind = 1    /// <lowbound查找;
    };

    /// accessor

    /// @brief   获取压缩的key; 用于iterator比较大小时使用; 避免一次内存Copy;
    /// @param   record_no, 记录的下标索引号;
    /// @param   base_index, 标识base串record No的数组下标;
    /// @param   prefix_combine_key, key的指针;
    /// @retval
    void GetPrefixCompressedKey(uint32_t record_no, uint32_t base_index,
                                PrefixCompressedKey* prefix_compressed_key) const;

    /// @brief   依据Record No获取key, 将key的值Copy到给定的Buffer当中;
    /// @param   record_no, 记录号
    /// @param   base_index, 标识base串record No的数组下标;
    /// @param   key,       指向key的Buffer指针;
    /// @param   key_len,   传入时为存放key的Buffer长度, 传出时为Key的实际长度;
    /// @retval  kRetOK, 成功;
    ///          kRetBufferOverflow, 内存不够;
    RetCode GetKeyByRecordNo(uint32_t record_no, uint32_t base_index,
                             char* key, uint16_t* key_len) const;

    void GetKeyByRecordNo(uint32_t record_no, uint32_t base_index,
                          std::string* key) const;

    /// @brief   获取val的指针;
    /// @param   record_no, 记录号
    /// @param   val,       输出参数, 存放值的Buffer指针的位置，
    /// @param   val_len，  输出参数, 传入时为存放key的Buffer长度, 传出时为Key的实际长度;
    void GetValPtrByRecordNo(uint32_t record_no, char** val, uint32_t* val_len) const;
    void GetValPtrByRecordNo(uint32_t record_no, StringPiece* val) const;

    /// @brief   获取val;   将值Copy到给定的Buffer当中;
    /// @param   record_no, 记录号
    /// @param   val,       存放值的Buffer，
    /// @param   val_len，  传入时为存放key的Buffer长度, 传出时为Key的实际长度;
    /// @retval  kRetOK, 成功;
    ///          kRetBufferOverflow, 内存不够;
    RetCode GetValByRecordNo(uint32_t record_no, void* val, uint32_t* val_len) const;
    void GetValByRecordNo(uint32_t record_no, std::string* val) const;

    /// @brief   获取key,val pair;
    /// @param   record_no, 记录号
    /// @param   base_index, 标识base串record No的数组下标;
    /// @param   key, key的指针;
    /// @param   key_len, key的长度;
    /// @param   val, 值的指针;
    /// @param   val_len; 值的长度;
    /// @retval  kRetOK, 成功;
    ///          kRetBufferOverflow, 内存不够;
    RetCode GetKVByRecordNo(uint32_t record_no, uint32_t base_index,
                            char* key, uint16_t* key_len,
                            char* val, uint32_t* val_len) const;

    /// @brief   获取下一个record的base_index;
    /// @param   cur_record_no, 当前的record_no;
    /// @param   cur_base_index, 当前的base_index;
    /// @retval  record_no+1的记录的base_index;
    uint32_t GetNextBaseIndex(uint32_t cur_record_no,
                              uint32_t cur_base_index) const;

    /// @brief    依照某个record的key在块中查找到块号;
    /// @param    key, 查询key的Buffer
    /// @param    key_len, 查询key len
    /// @param    record_no, 需要查找的record No;
    /// @param    base_index, 标识base串record No 的数组下标;
    /// @param    flag: kAccurateFind = 0, /// <精确查找，
    ///                 kLowBoundFind = 1  /// <lowbound查找，
    /// @param    is_accurated_hit, 在low_bound查找时，返回是否精确命中;
    /// @retval   kRetOK:            成功
    ///           kRetRecordNotFind: Not find
    RetCode FindRecordNoByKey(const void* key, uint16_t key_len,
                              uint32_t* record_no, uint32_t* base_index,
                              uint32_t flag, bool* is_accurated_hit) const;

    /// @brief   将文件中读取出来的数据解析到Block的Buffer当中;
    /// @param   buffer, 输入的buffer
    /// @param   buffer_len, 输入的buffer len
    /// @retval  kRetOK, 成功;
    ///          kRetUnCompressError, 解压失败;
    RetCode UnCompressBlockData(const void* buffer, uint32_t buffer_len);

    /// @brief   解析内存当中的非压缩Block;
    /// @param   buffer, 输入的buffer;
    /// @param   buffer_len, 输入的buffer len;
    /// @retval
    void ParseBlockData(const char* buffer, uint32_t buffer_len);

protected:

    /// @brief   添加引用计数;
    ///          外面已经加锁, 不用加锁,
    /// @param   无
    /// @retval  引用计数的值;
    uint32_t IncRef()
    {
        m_ref++;
        return m_ref;
    }

    /// @brief   减少引用计数;
    ///          外面已经加锁, 不用加锁,
    /// @param   无
    /// @retval  引用计数的值;
    uint32_t DecRef()
    {
        m_ref--;
        return m_ref;
    }

    // accessor and mutator for reader;
    void SetMemoryFromCache()
    {
        m_memory_from_cache = true;
    }

    bool IsMemoryFromCache()
    {
        return m_memory_from_cache;
    }

    /// @brief   重置成员Buffer为空,
    ///          并且将buffer的指针和长度取出,注意、此处不会删除Buffer;
    /// @param   buffer;
    /// @retval  无;
    void SwapOutBuffer(char** buffer, uint32_t* buffer_len)
    {
        assert((m_buffer != NULL) && (m_buffer_len != 0));
        assert((buffer != NULL) && (buffer_len != NULL));
        (*buffer) = m_buffer;
        (*buffer_len) = m_buffer_len;
        m_buffer = NULL;
        m_buffer_len = 0;
    }

private:

    /// @brief  固定长度key的查找;
    /// @param  key, 查询key的Buffer
    /// @param  key_len, 查询key len
    /// @param  record_no, 需要查找的record No;
    /// @param  base_index, 标识base串record No 的数组下标;
    /// @param  flag:
    ///               kAccurateFind = 0, /// <精确查找，
    ///               kLowBoundFind = 1  /// <lowbound查找,
    /// @param   is_accurated_hit, 在low_bound查找时，返回是否精确命中;
    /// @retval  kRetOK:            成功
    ///          kRetRecordNotFind: Not find
    RetCode FixedKeyLenFind(const void* key, uint16_t key_len, uint32_t* record_no,
                            uint32_t* base_index, uint32_t flag, bool* is_accurated_hit) const;

    /// @brief  Key的二分查找;
    /// @param  key, key的buffer指针;
    /// @param  key_len, key的长度;
    /// @param  record_no,  输出该块当中的记录号;
    /// @param  flag,
    /// @param   is_accurated_hit, 在low_bound查找时，返回是否精确命中;
    /// @retval true, 找到;
    ///         false, 找不到;
    bool FixedKeyLenBinarySearch(const void* key, uint16_t key_len, uint32_t* record_no,
                                 uint32_t flag, bool* is_accurated_hit) const;

    /// @brief   变长度key的查找;
    /// @param   key, 查询key的Buffer
    /// @param   key_len, 查询key len
    /// @param   record_no, 需要查找的record No;
    /// @param   base_index, 标识base串record No 的数组下标;
    /// @param   flag:
    ///               kAccurateFind = 0, /// <精确查找,
    ///               kLowBoundFind = 1  /// <lowbound查找,
    /// @param   is_accurated_hit, 在low_bound查找时，返回是否精确命中;
    /// @retval  kRetOK:            成功
    ///          kRetRecordNotFind: Not find
    RetCode VarKeyLenFind(const void* key, uint16_t key_len, uint32_t* record_no,
                          uint32_t* base_index, uint32_t flag, bool* is_accurated_hit) const;

    /// @brief   将文件中读取出来的数据解析到Block的Buffer当中;
    /// @param   buffer, 输入的buffer
    /// @param   buffer_len, 输入的buffer len
    /// @retval  kRetOK, 成功;
    ///          kRetUnCompressError, 解压失败;
    RetCode UnCompressBlockHeader(const char* buffer, uint32_t buffer_len);

    /// @brief   将文件中读取出来的数据解析到Block的Buffer当中;
    /// @param   w_buffer, 输出的buffer
    /// @param   r_buffer, 输入的buffer
    /// @param   w_len,    输出的buffer len
    /// @param   r_len,    输入的buffer len
    /// @retval  kRetOK, 成功;
    ///          kRetUnCompressError, 解压失败;
    void UnCompressOffset(char* w_buffer, const char* r_buffer,
                             uint32_t* w_len, uint32_t* r_len);

    uint32_t        m_ref;      /// <block的引用计数;
    bool            m_memory_from_cache; /// <标识块里面的内存是否是来自Cache;
};

} // namespace sstable

#endif // SSTABLE_SSTABLE_READER_BLOCK_H_

