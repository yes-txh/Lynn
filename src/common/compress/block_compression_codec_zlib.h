/**
 * @file BlockCompressionCodecZlib.h
 * @brief
 * @author kypoyin
 * @date 2010-04-29
 */

#ifndef BLOCK_COMPRESSION_CODEC_ZLIB_H__
#define BLOCK_COMPRESSION_CODEC_ZLIB_H__

#include <zlib.h>
#include "common/base/stdint.h"
#include "common/compress/block_compression_codec.h"

namespace intern {

class BlockCompressionCodecZlib:public BlockCompressionCodec {
public:
    BlockCompressionCodecZlib();
    virtual ~BlockCompressionCodecZlib();

    /**
     * @brief ����ѹ��������zlib��֧��"best","9","normal"��������
     *
     * @param name ��������
     * @param value ����ֵΪNULL
     */
    void SetArgs(const char *args,  const char *value = NULL);

    int GetType() {return ZLIB;}

private:
    virtual int DoDeflate(const char *input,
                          size_t input_size,
                          char *output,
                          size_t *output_size);

    virtual int DoInflate(const char *input,
                          size_t input_size,
                          char *output,
                          size_t *output_size);

    z_stream m_stream_inflate;
    bool m_inflate_initialized;

    z_stream m_stream_deflate;
    bool m_deflate_initialized;

    int  m_level;
};

}

#endif  // BLOCK_COMPRESSION_CODEC_ZLIB_H__

