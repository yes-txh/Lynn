/**
 * @file  BlockCompressionCodecManager.h
 * @brief 压缩对象管理类
 * @version 1.0
 * @date    2010-07-12 20时57分46秒
 * @author  Kypoyin, kypoyin@tencent.com
 */

#ifndef BLOCK_COMPRESSION_CODEC_MANAGER_H__
#define BLOCK_COMPRESSION_CODEC_MANAGER_H__

#include  "common/system/concurrency/mutex.hpp"
#include  "common/compress/block_compression_codec.h"

namespace intern {

class BlockCompressionCodecManager {
public:
    BlockCompressionCodecManager(int max_free_codec = 5);
    ~BlockCompressionCodecManager();

    BlockCompressionCodec *CreateBlockCompressionCodec(int codec_type);
    void DestroyBlockCompressionCodec(BlockCompressionCodec *codec);

private:
    // 每种类型的压缩对象的常驻内存的最大限制
    int m_max_free_codec_size;
    int m_free_codec_size_array[BlockCompressionCodec::COMPRESSION_TYPE_LIMIT];

    // 压缩对象链表的两维数组
    BlockCompressionCodec **m_codec_array;
    SimpleMutex m_mutex;
};

}
#endif  // BLOCK_COMPRESSION_CODEC_MANAGER_H__
