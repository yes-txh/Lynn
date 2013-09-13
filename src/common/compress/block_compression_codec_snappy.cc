/**
 * @brief
 * @author huanyu
 * @date 2011/04/29
 */

#include "common/compress/block_compression_codec_snappy.h"
#include "thirdparty/snappy/snappy.h"

namespace intern {
BlockCompressionCodecSnappy::BlockCompressionCodecSnappy() { }

int BlockCompressionCodecSnappy::DoDeflate(const char *input,
                                           size_t input_size,
                                           char *output,
                                           size_t *output_size) {
    if (input_size > *output_size) {
        return COMPRESSION_E_OUTPUT_OVERRUN;
    }

    snappy::RawCompress(input, input_size, output, output_size);
    return COMPRESSION_E_OK;
}

int BlockCompressionCodecSnappy::DoInflate(const char *input,
                                           size_t input_size,
                                           char *output,
                                           size_t *output_size) {
    if (!snappy::GetUncompressedLength(input, input_size, output_size)) {
        return COMPRESSION_E_UNPACK_LEN_ERROR;
    }
    snappy::RawUncompress(input, input_size, output);
    return COMPRESSION_E_OK;
}

}


