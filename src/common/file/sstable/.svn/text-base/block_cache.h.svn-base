#ifndef SSTABLE_BLOCK_CACHE_H_
#define SSTABLE_BLOCK_CACHE_H_

#include <vector>
#include <queue>
#include "common/base/stdext/hash_map.hpp"
#include "common/base/stdint.h"
#include "common/system/concurrency/mutex.hpp"
#include "thirdparty/gtest/gtest.h"

namespace sstable
{
class BlockCache
{
public:

    friend class BlockCacheTest;
    FRIEND_TEST(BlockCacheTest, PushBlock);
    FRIEND_TEST(BlockCacheTest, FetchBlock);
    FRIEND_TEST(BlockCacheTest, Existed);
    FRIEND_TEST(BlockCacheTest, ReleaseBlock);
    FRIEND_TEST(BlockCacheTest, ReclaimBlocks);
    FRIEND_TEST(BlockCacheTest, ComputePriority);

    // 构造和析构
    explicit BlockCache(uint64_t max_cached_bytes,
                        uint32_t max_cached_blocks_num,
                        uint32_t reclaimed_bytes_threshold);

    ~BlockCache();

    static BlockCache* GetCache()
    {
        return s_sstable_block_cache;
    }

    /// @brief 初始化全局的静态Cache;
    /// @param max_cached_bytes, 最大cache的字节数;
    /// @param max_cached_blocks_num, 最大cache的块数;
    /// @param reclaimed_bytes_threshold
    static void InitCache(uint64_t max_cached_bytes,
                          uint32_t max_cached_blocks_num,
                          uint32_t reclaimed_bytes_threshold)
    {
        if (s_sstable_block_cache == NULL)
        {
            s_sstable_block_cache = new BlockCache(max_cached_bytes,
                                                   max_cached_blocks_num,
                                                   reclaimed_bytes_threshold);
        }
    }

    /// @brief  释放全局的静态Cache;
    static  void DestroyCache()
    {
        delete s_sstable_block_cache;
        s_sstable_block_cache = NULL;
    }

    // accessor and mutator
    double GetHitRatio() const
    {
        if (m_try_access_num == 0)
            return static_cast<double>(0);
        else
            return static_cast<double>(m_hit_num) / m_try_access_num;
    }

protected:

    friend class SSTableReader;

    /// @brief  向Cache当中push一个Block的内存块; 若内存块在Cache当中已经存在时，
    ///         不做插入操作, 返回成功;
    /// @param  file_id,      sstable文件的id号,
    /// @param  block_no,     block在sstable文件中的block号;
    /// @param  block_buffer, block的buffer;
    /// @param  buffer_len,   buffer的长度;
    /// @retval true: 成功; 不存在时，插入成功，存在时，不做插入操作, 返回成功;
    ///        false: 失败, 不存在，并且Cache已经满(回收完之后还满);
    bool PushBlock(uint64_t file_id, uint16_t block_no,
                   const char* block_buffer, uint32_t buffer_len);

    /// @brief 在Cache当中查找Block块; 找到时,会增加引用计数;
    /// @param file_id      sstable文件的id号;
    /// @param block_no     block在sstable文件中的block号;
    /// @param block_buffer block的buffer指针;
    /// @param buffer_len   block的长度;
    /// @retval true, 获取成功;
    ///        false, 获取失败;
    bool FetchBlock(uint64_t file_id, uint16_t block_no,
                    char** block_buffer, uint32_t* buffer_len);

    /// @brief 查询某个file的block是否在cache当中存在;
    /// @param file_id
    /// @param block_no
    /// @retval true, 存在;
    ///        false, 不存在;
    bool Existed(uint64_t file_id, uint16_t block_no);

    /// @brief 对cache中的一个block进行引用计数的减操作;
    ///       引用计数<0时，需要删除该Cache项;
    /// @param file_id, 文件id;
    /// @param block_no, 块号
    void ReleaseBlock(uint64_t file_id, uint16_t block_no);

private:

    static const uint64_t kMilisecondsPerMin = 1000 * 60;

    /// @brief 回收内存;
    /// @param cur_time, 当前时间;
    void ReclaimBlocks(uint64_t cur_time);

#pragma pack(push, 1)
    struct CachedUnit
    {
        CachedUnit(): block_buffer(NULL),
            buffer_len(0),
            num_hits(0),
            first_access_time(0),
            file_id(0),
            block_no(0),
            priority(0),
            fetch_reference(0)
        {}
        char*      block_buffer;       /// 指向block buffer的指针;
        uint32_t   buffer_len;         /// buffer的长度;
        uint64_t   num_hits;           /// 有效访问次数;
        uint64_t   first_access_time;  /// 有效访问次数中第一次访问的时间;
        uint64_t   file_id;            /// sstable的文件id号;
        uint16_t   block_no;           /// block号;
        double     priority;           /// 淘汰时使用的优先权;
        int64_t    fetch_reference;    /// block内存的引用计数;
    };
#pragma pack(pop)

    typedef stdext::hash_map<uint16_t, CachedUnit* > BlockMap;
    typedef stdext::hash_map<uint64_t, BlockMap* >   SSTableBlockMap;

    // 用于构建最小堆的比较函数;
    struct CachedUnitPtrCmp
    {
        bool operator()(const CachedUnit* unit_ptr1, const CachedUnit* unit_ptr2)
        {
            assert((unit_ptr1 != NULL) && (unit_ptr2 != NULL));
            return unit_ptr1->priority > unit_ptr2->priority;
        }
    };

    typedef std::priority_queue < CachedUnit*,
            std::vector<CachedUnit*>, CachedUnitPtrCmp >  ReclaimHeap;

    /// @breif 计算优先权;
    /// @param
    /// @param
    void ComputePriority(CachedUnit* unit, uint64_t cur_time);

    uint32_t  m_num_cached_blocks; /// cached block数目;
    uint64_t  m_cached_bytes; /// cached bytes;

    uint64_t  m_try_access_num; /// 尝试读取的次数;
    uint64_t  m_hit_num; /// 命中的次数;

    uint64_t  m_max_cached_bytes; /// 最大cached的字节数;
    uint64_t  m_max_cached_block_num; /// 最大cached的块数;
    uint64_t  m_reclaim_threshold; /// 每次进行内存回收需要回收的块数目;

    SSTableBlockMap  m_blocks;  /// 存放CachedUnit的哈希表;
    std::queue<CachedUnit*> m_free_units_queue; /// 空闲的unit对象的队列;
    char*     m_cached_units_memory; /// CachedUnit的连续内存空间;
    RecursiveMutex     m_mutex; /// 线程锁;

    static    BlockCache*  s_sstable_block_cache;
};

} // namespace sstable

#endif // SSTABLE_BLOCK_CACHE_H_
