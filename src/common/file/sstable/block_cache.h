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

    // ���������
    explicit BlockCache(uint64_t max_cached_bytes,
                        uint32_t max_cached_blocks_num,
                        uint32_t reclaimed_bytes_threshold);

    ~BlockCache();

    static BlockCache* GetCache()
    {
        return s_sstable_block_cache;
    }

    /// @brief ��ʼ��ȫ�ֵľ�̬Cache;
    /// @param max_cached_bytes, ���cache���ֽ���;
    /// @param max_cached_blocks_num, ���cache�Ŀ���;
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

    /// @brief  �ͷ�ȫ�ֵľ�̬Cache;
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

    /// @brief  ��Cache����pushһ��Block���ڴ��; ���ڴ����Cache�����Ѿ�����ʱ��
    ///         �����������, ���سɹ�;
    /// @param  file_id,      sstable�ļ���id��,
    /// @param  block_no,     block��sstable�ļ��е�block��;
    /// @param  block_buffer, block��buffer;
    /// @param  buffer_len,   buffer�ĳ���;
    /// @retval true: �ɹ�; ������ʱ������ɹ�������ʱ�������������, ���سɹ�;
    ///        false: ʧ��, �����ڣ�����Cache�Ѿ���(������֮����);
    bool PushBlock(uint64_t file_id, uint16_t block_no,
                   const char* block_buffer, uint32_t buffer_len);

    /// @brief ��Cache���в���Block��; �ҵ�ʱ,���������ü���;
    /// @param file_id      sstable�ļ���id��;
    /// @param block_no     block��sstable�ļ��е�block��;
    /// @param block_buffer block��bufferָ��;
    /// @param buffer_len   block�ĳ���;
    /// @retval true, ��ȡ�ɹ�;
    ///        false, ��ȡʧ��;
    bool FetchBlock(uint64_t file_id, uint16_t block_no,
                    char** block_buffer, uint32_t* buffer_len);

    /// @brief ��ѯĳ��file��block�Ƿ���cache���д���;
    /// @param file_id
    /// @param block_no
    /// @retval true, ����;
    ///        false, ������;
    bool Existed(uint64_t file_id, uint16_t block_no);

    /// @brief ��cache�е�һ��block�������ü����ļ�����;
    ///       ���ü���<0ʱ����Ҫɾ����Cache��;
    /// @param file_id, �ļ�id;
    /// @param block_no, ���
    void ReleaseBlock(uint64_t file_id, uint16_t block_no);

private:

    static const uint64_t kMilisecondsPerMin = 1000 * 60;

    /// @brief �����ڴ�;
    /// @param cur_time, ��ǰʱ��;
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
        char*      block_buffer;       /// ָ��block buffer��ָ��;
        uint32_t   buffer_len;         /// buffer�ĳ���;
        uint64_t   num_hits;           /// ��Ч���ʴ���;
        uint64_t   first_access_time;  /// ��Ч���ʴ����е�һ�η��ʵ�ʱ��;
        uint64_t   file_id;            /// sstable���ļ�id��;
        uint16_t   block_no;           /// block��;
        double     priority;           /// ��̭ʱʹ�õ�����Ȩ;
        int64_t    fetch_reference;    /// block�ڴ�����ü���;
    };
#pragma pack(pop)

    typedef stdext::hash_map<uint16_t, CachedUnit* > BlockMap;
    typedef stdext::hash_map<uint64_t, BlockMap* >   SSTableBlockMap;

    // ���ڹ�����С�ѵıȽϺ���;
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

    /// @breif ��������Ȩ;
    /// @param
    /// @param
    void ComputePriority(CachedUnit* unit, uint64_t cur_time);

    uint32_t  m_num_cached_blocks; /// cached block��Ŀ;
    uint64_t  m_cached_bytes; /// cached bytes;

    uint64_t  m_try_access_num; /// ���Զ�ȡ�Ĵ���;
    uint64_t  m_hit_num; /// ���еĴ���;

    uint64_t  m_max_cached_bytes; /// ���cached���ֽ���;
    uint64_t  m_max_cached_block_num; /// ���cached�Ŀ���;
    uint64_t  m_reclaim_threshold; /// ÿ�ν����ڴ������Ҫ���յĿ���Ŀ;

    SSTableBlockMap  m_blocks;  /// ���CachedUnit�Ĺ�ϣ��;
    std::queue<CachedUnit*> m_free_units_queue; /// ���е�unit����Ķ���;
    char*     m_cached_units_memory; /// CachedUnit�������ڴ�ռ�;
    RecursiveMutex     m_mutex; /// �߳���;

    static    BlockCache*  s_sstable_block_cache;
};

} // namespace sstable

#endif // SSTABLE_BLOCK_CACHE_H_
