#ifndef SSTABLE_SSTABLE_BLOCK_H_
#define SSTABLE_SSTABLE_BLOCK_H_

#include <assert.h>
#include "common/file/sstable/sstable_def.h"
#include "common/file/sstable/sstable_header.pb.h"
#include "common/file/sstable/sstable_block_header.pb.h"

namespace sstable
{
class Block
{
public:
    /// 构造和析构
    explicit Block(const SSTableHeader* header): m_buffer(NULL),
        m_buffer_len(0),
        m_sstable_header(const_cast<SSTableHeader*>(header)),
        m_block_header(NULL),
        m_key_common_prefix(NULL),
        m_key_start_ptr(NULL),
        m_key_offset_ptr(NULL),
        m_basekey_index_ptr(NULL),
        m_val_offset_ptr(NULL),
        m_val_ptr(NULL)
    {}

    virtual ~Block()
    {
        if (m_buffer != NULL)
        {
            delete[]m_buffer;
            m_buffer = NULL;
        }
        if (m_block_header != NULL)
        {
            delete m_block_header;
            m_block_header = NULL;
        }
    }

    /// @brief   获取块中记录数目
    /// @params  无,
    /// @retval  块中的记录数;
    inline uint32_t GetNumRecords() const
    {
        assert(m_block_header != NULL);
        return m_block_header->num_records();
    }

protected:
    char*           m_buffer;               ///< 指向存放Block数据的指针;
    uint32_t        m_buffer_len;           ///< block数据的长度;
    SSTableHeader*  m_sstable_header;       ///< 指向SSTable头的指针;

    BlockHeader*    m_block_header;         ///< 指向BlockHeader的指针;
    char*           m_key_common_prefix;    ///< key的最大公共子串的地址;
    char*           m_key_start_ptr;        ///< Key区域的起始指针;
    uint32_t*       m_key_offset_ptr;       ///< Key,Offset区域的起始指针;
    uint32_t*       m_basekey_index_ptr;    ///< basekey的index区域的指针;
    uint32_t*       m_val_offset_ptr;       ///< Data的offset区域的指针;
    char*           m_val_ptr;              ///< 数据区的起始指针;
};

} // namespace sstable

#endif // SSTABLE_SSTABLE_BLOCK_H_
