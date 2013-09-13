/**
 * @brief
 * @author huanyu
 * @date 2011/04/29
 */

#ifndef BLOCK_COMPRESSION_CODEC_SNAPPY_H__
#define BLOCK_COMPRESSION_CODEC_SNAPPY_H__

#include "common/compress/block_compression_codec.h"

namespace intern {

class BlockCompressionCodecSnappy : public BlockCompressionCodec {
public:
    BlockCompressionCodecSnappy();

    virtual int GetType() { return SNAPPY; }
private:
    virtual int DoDeflate(const char *input,
                          size_t input_size,
                          char *output,
                          size_t *output_size);

    virtual int DoInflate(const char *input,
                          size_t input_size,
                          char *output,
                          size_t *output_size);

};

}

#endif  // BLOCK_COMPRESSION_CODEC_SNAPPY_H__
