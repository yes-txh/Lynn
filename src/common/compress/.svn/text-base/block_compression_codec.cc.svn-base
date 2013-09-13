/**
 * @file BlockCompressionCodec.cc
 * @brief
 * @author kypoyin
 * @date 2010-05-05
 */

#include "common/compress/block_compression_codec.h"
#include "common/compress/block_compression_codec_none.h"
#include "common/compress/block_compression_codec_zlib.h"
#include "common/compress/block_compression_codec_bmz.h"
#include "common/compress/block_compression_codec_lzo.h"
#include "common/compress/block_compression_codec_quicklz.h"
#include "common/compress/block_compression_codec_snappy.h"
#include "common/compress/block_compression_codec_manager.h"
#include "common/compress/checksum/checksum.h"

namespace intern {

BlockCompressionCodec *BlockCompressionCodec::CreateCodec(int type) {
    switch (type) {
        case BlockCompressionCodec::NONE :
            return new BlockCompressionCodecNone();

        case BlockCompressionCodec::ZLIB :
            return new BlockCompressionCodecZlib();

        case BlockCompressionCodec::BMZ :
            return new BlockCompressionCodecBmz();

        case BlockCompressionCodec::LZO :
            return new BlockCompressionCodecLzo();

        case BlockCompressionCodec::QUICKLZ :
            return new BlockCompressionCodecQuicklz();

        case BlockCompressionCodec::SNAPPY :
            return new BlockCompressionCodecSnappy();

        default:
            break;
    }

    return NULL;
}

int BlockCompressionCodec::Deflate(const char *input,
                                   size_t input_size,
                                   char *output,
                                   size_t &output_size,
                                   uint32_t &crc) {
    int ret = DoDeflate(input, input_size, output, &output_size);
    if (ret != COMPRESSION_E_OK) {
        return ret;
    }
    crc = fletcher32(output, output_size);
    return COMPRESSION_E_OK;
}

int BlockCompressionCodec::Inflate(const char *input,
                                   size_t input_size,
                                   char *output,
                                   size_t &output_size,
                                   uint32_t crc) {
    // 计算校验码
    if (crc != fletcher32(input, input_size)) {
        return COMPRESSION_E_CHECKSUM_ERROR;
    }

    return DoInflate(input, input_size, output, &output_size);
}

int BlockCompressionCodec::Deflate(const char *input,
                                   size_t input_size,
                                   char *output,
                                   size_t *output_size) {
    return DoDeflate(input, input_size, output, output_size);
}

int BlockCompressionCodec::Inflate(const char *input,
                                   size_t input_size,
                                   char *output,
                                   size_t *output_size) {
    return DoInflate(input, input_size, output, output_size);
}

}

