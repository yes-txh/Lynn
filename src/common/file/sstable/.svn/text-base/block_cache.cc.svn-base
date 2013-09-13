
#include "common/system/time/time_utils.hpp"
#include "common/file/sstable/block_cache.h"
#include "thirdparty/glog/logging.h"

namespace sstable
{
BlockCache* BlockCache::s_sstable_block_cache = NULL;
// 构造和析构
BlockCache::BlockCache(uint64_t max_cached_bytes,
                     uint32_t max_cached_blocks_num,
                     uint32_t reclaimed_blocks_threshold):
    m_num_cached_blocks(0),
    m_cached_bytes(0),
    m_try_access_num(0),
    m_hit_num(0),
    m_max_cached_bytes(max_cached_bytes),
    m_max_cached_block_num(max_cached_blocks_num),
    m_reclaim_threshold(reclaimed_blocks_threshold),
    m_cached_units_memory(NULL)
{
    m_cached_units_memory = new char[max_cached_blocks_num*sizeof(CachedUnit)];

    char* tmp_ptr = m_cached_units_memory;
    for (uint32_t count_units =0; count_units < max_cached_blocks_num; count_units++) {
        m_free_units_queue.push(reinterpret_cast<CachedUnit*>(tmp_ptr));
        tmp_ptr += sizeof(CachedUnit);
    }
}

BlockCache::~BlockCache()
{
    // 1: 清除所有block buffer的内存;
    SSTableBlockMap::iterator map_iter = m_blocks.begin();
    while (map_iter != m_blocks.end()) {
        BlockMap* cur_block_map = map_iter->second;
        BlockMap::iterator block_iter = cur_block_map->begin();
        while (block_iter != cur_block_map->end()) {
            CachedUnit* cur_unit = block_iter->second;
            delete [] cur_unit->block_buffer;
            cur_unit->block_buffer = NULL;
            block_iter++;
        }
        delete cur_block_map;
        map_iter->second = NULL;
        map_iter++;
    }

    // 2: 清除cacheunits的内存;
    delete [] m_cached_units_memory;
}

/// @brief  向Cache当中push一个Block的内存块; 若内存块在Cache当中已经存在时，
///        不做插入操作, 返回成功;
/// @param  file_id,      sstable文件的id号,
/// @param  block_no,     block在sstable文件中的block号;
/// @param  block_buffer, block的buffer;
/// @param  buffer_len,   buffer的长度;
/// @retval true: 成功; 不存在时，插入成功，存在时，不做插入操作, 返回成功;
///        false: 失败, 不存在，并且Cache已经满(回收完之后还满);
bool BlockCache::PushBlock(uint64_t file_id, uint16_t block_no,
               const char* block_buffer, uint32_t buffer_len)
{
    bool b_ret = true;

    assert(file_id != 0);
    assert((block_buffer != NULL) && (buffer_len != 0));

    MutexLocker locker(&m_mutex);
    uint64_t cur_time = TimeUtils::Milliseconds();

    if (buffer_len > m_max_cached_bytes) {
        // 一个块的大小超过最大值;
        LOG(ERROR) << "one block size overflow, current push bytes:" << buffer_len;
        return false;
    }

    // 判断cache是否已经满;
    if (!Existed(file_id, block_no) &&
        (((m_cached_bytes + buffer_len) > m_max_cached_bytes)
        || (m_num_cached_blocks == m_max_cached_block_num))) {
            // cache已经满, 需要删除和回收内存;
            ReclaimBlocks(cur_time);

            if (((m_cached_bytes + buffer_len) > m_max_cached_bytes)
                || (m_num_cached_blocks == m_max_cached_block_num)) {
                    LOG(ERROR) << "cache full after reclaim, cached bytes:" << m_cached_bytes
                        << "current push bytes:" << buffer_len;
                    return false;
            }
    }

    // 查找到该文件的block map;
    BlockMap* cur_file_block_map = NULL;
    SSTableBlockMap::iterator map_iter = m_blocks.find(file_id);
    if (map_iter == m_blocks.end()) {
        cur_file_block_map = new BlockMap();
        m_blocks[file_id] = cur_file_block_map;
    } else {
        cur_file_block_map = map_iter->second;
    }

    // 查找cache unit;
    assert(cur_file_block_map != NULL);
    BlockMap::iterator block_iter = cur_file_block_map->find(block_no);
    if (block_iter == cur_file_block_map->end()) {
        CachedUnit* cur_unit = m_free_units_queue.front();
        assert(cur_unit != NULL);
        cur_unit->block_buffer = const_cast<char*>(block_buffer);
        cur_unit->block_no = block_no;
        cur_unit->buffer_len = buffer_len;
        cur_unit->file_id = file_id;
        cur_unit->first_access_time = cur_time;
        cur_unit->num_hits = 1;
        cur_unit->fetch_reference = 0;

        (*cur_file_block_map)[block_no] = cur_unit;
        m_cached_bytes += buffer_len;
        m_num_cached_blocks++;
        m_free_units_queue.pop();
    }

    return b_ret;
}

/// @brief 在Cache当中查找Block块;
/// @param file_id      sstable文件的id号;
/// @param block_no     block在sstable文件中的block号;
/// @param block_buffer block的buffer指针;
/// @param buffer_len   block的长度;
/// @retval true, 获取成功;
///        false, 获取失败;
bool BlockCache::FetchBlock(uint64_t file_id, uint16_t block_no,
                char** block_buffer, uint32_t* buffer_len)
{
    bool b_ret = false;

    assert(file_id != 0);
    assert((block_buffer != NULL) && (buffer_len != NULL));

    MutexLocker locker(&m_mutex);
    m_try_access_num++;


    SSTableBlockMap::iterator map_iter = m_blocks.find(file_id);
    if (map_iter == m_blocks.end()) {
        return false;
    }

    BlockMap* hited_file_block_map = map_iter->second;
    assert(hited_file_block_map != NULL);

    BlockMap::iterator block_iter = hited_file_block_map->find(block_no);

    if (block_iter != hited_file_block_map->end()) {
        CachedUnit* cur_unit = block_iter->second;
        assert(cur_unit != NULL);
        assert((cur_unit->block_buffer != NULL)
            && (cur_unit->buffer_len != 0));

        cur_unit->num_hits++;
        cur_unit->fetch_reference++;
        *block_buffer = cur_unit->block_buffer;
        *buffer_len = cur_unit->buffer_len;
        m_hit_num++;
        b_ret = true;
    }

    return b_ret;
}

/// @brief 回收内存;
void BlockCache::ReclaimBlocks(uint64_t cur_time)
{
    // 1: 遍历map构建最小优先权堆;
    ReclaimHeap  reclaim_heap;

    SSTableBlockMap::iterator map_iter = m_blocks.begin();
    while (map_iter != m_blocks.end()) {
        BlockMap* cur_file_block_map = map_iter->second;
        assert(cur_file_block_map != NULL);
        BlockMap::iterator block_iter = cur_file_block_map->begin();
        while (block_iter != cur_file_block_map->end()) {
            CachedUnit* cur_unit = block_iter->second;
            assert(cur_unit != NULL);

            if (cur_unit->fetch_reference == 0) {
                // 回收引用计数为0的块;
                ComputePriority(cur_unit, cur_time);
                if (reclaim_heap.size() < m_reclaim_threshold) {
                    // 构建大小为m_reclaim_threshold的堆;
                    reclaim_heap.push(cur_unit);
                } else {
                    // 堆已经满; 先push再pop;
                    reclaim_heap.push(cur_unit);
                    reclaim_heap.pop();
                }
            } // if
            block_iter++;
        } // while block_iter
        map_iter++;
    } // while map_iter

    // 2: 对堆中保留的unit进行删除;
    while (!reclaim_heap.empty()) {
        CachedUnit* top_unit = reclaim_heap.top();
        ReleaseBlock(top_unit->file_id, top_unit->block_no);
        reclaim_heap.pop();
    }
}

/// @brief 查询某个file的block是否在cache当中存在;
/// @param file_id
/// @param block_no
/// @retval true, 存在;
///        false, 不存在;
bool BlockCache::Existed(uint64_t file_id, uint16_t block_no)
{
    MutexLocker locker(m_mutex);
    SSTableBlockMap::iterator map_iter = m_blocks.find(file_id);

    if (map_iter == m_blocks.end()) return false;

    BlockMap* hited_file_block_map = map_iter->second;
    assert(hited_file_block_map != NULL);

    return (hited_file_block_map->find(block_no) != hited_file_block_map->end());
}

/// @brief 对cache中的一个block进行引用计数的减操作;
///       单引用计数为0时, 删除掉该block的cache;
///       调用该函数时, 必然文件id和block必然存在;
/// @param file_id, 文件id;
/// @param block_no, 块号
void BlockCache::ReleaseBlock(uint64_t file_id, uint16_t block_no)
{
    assert(file_id != 0);
    MutexLocker locker(&m_mutex);

    SSTableBlockMap::iterator map_iter = m_blocks.find(file_id);
    assert(map_iter != m_blocks.end());

    BlockMap* hited_file_block_map = map_iter->second;
    assert(hited_file_block_map != NULL);
    BlockMap::iterator block_iter = hited_file_block_map->find(block_no);
    assert(block_iter != hited_file_block_map->end());

    CachedUnit* cur_unit = block_iter->second;
    assert((cur_unit != NULL) && (cur_unit->block_buffer != NULL));
    cur_unit->fetch_reference--;

    if (cur_unit->fetch_reference < 0)
    {
        m_cached_bytes -= cur_unit->buffer_len;
        m_num_cached_blocks--;

        delete [](cur_unit->block_buffer);
        cur_unit->block_buffer = NULL;

        m_free_units_queue.push(cur_unit);
        block_iter->second = NULL;

        hited_file_block_map->erase(block_iter);

        if (hited_file_block_map->empty())
        {
            delete hited_file_block_map;
            map_iter->second = NULL;
            m_blocks.erase(map_iter);
        }
    }
}

/// @breif 计算优先权;
/// @param
/// @param
void BlockCache::ComputePriority(CachedUnit* unit, uint64_t cur_time)
{
    uint64_t elapse_min = (cur_time - unit->first_access_time) / kMilisecondsPerMin;

    if (elapse_min == 0)  // 防止除0操作;
    {
        elapse_min = 1;
    }

    unit->priority = static_cast<double>(unit->num_hits) / elapse_min;
}

} // namespace sstable
