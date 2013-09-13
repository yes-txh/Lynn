/**
 * @file BlockCompressionCodecZlib.cc
 * @brief
 * @author kypoyin
 * @date 2010-04-29
 */

#include <string.h>
#include <stdint.h>
#include "common/compress/checksum/checksum.h"
#include "common/compress/block_compression_codec_zlib.h"

namespace intern {

BlockCompressionCodecZlib::BlockCompressionCodecZlib()
    : m_inflate_initialized(false),
      m_deflate_initialized(false),
      m_level(Z_BEST_SPEED) {
}

BlockCompressionCodecZlib::~BlockCompressionCodecZlib() {
    if (m_deflate_initialized) {
        deflateEnd(&m_stream_deflate);
    }
    if (m_inflate_initialized) {
        inflateEnd(&m_stream_inflate);
    }
}

void BlockCompressionCodecZlib::SetArgs(const char *name, const char *value) {
    bool set_level = false;
    if ((0 == strcasecmp(name, "base") || 0 == strcasecmp(name, "9"))
        && (m_level != Z_BEST_SPEED)) {
        set_level = true;
        m_level = Z_BEST_SPEED;
    } else if (0 == strcasecmp(name, "normal") &&
               m_level != Z_DEFAULT_COMPRESSION) {
        set_level = true;
        m_level = Z_DEFAULT_COMPRESSION;
    }

    if (set_level && m_deflate_initialized) {
        deflateEnd(&m_stream_deflate);
        m_deflate_initialized = false;
    }
}


int BlockCompressionCodecZlib::DoDeflate(const char *input,
                                         size_t input_size,
                                         char *output,
                                         size_t *output_size) {
    // zlib docs in http://www.zlib.net/zlib_tech.html

    int ret;
    if (!m_deflate_initialized) {
        memset(&m_stream_deflate, 0, sizeof(m_stream_deflate));
        m_stream_deflate.zalloc = Z_NULL;
        m_stream_deflate.zfree = Z_NULL;
        m_stream_deflate.opaque = Z_NULL;
        ret = deflateInit(&m_stream_deflate, m_level);
        if(ret != Z_OK){
            return COMPRESSION_E_PACK_ERROR;
        }

        m_deflate_initialized = true;
    }

    m_stream_deflate.avail_in = input_size;
    m_stream_deflate.next_in = (Bytef*)input;

    m_stream_deflate.avail_out = (*output_size);
    m_stream_deflate.next_out = (Bytef*)(output);

    ret = ::deflate(&m_stream_deflate, Z_FINISH);
    if (ret != Z_STREAM_END) {
        return COMPRESSION_E_PACK_ERROR;
    }


    *output_size -= m_stream_deflate.avail_out;

    deflateReset(&m_stream_deflate);
    return COMPRESSION_E_OK;
}

int BlockCompressionCodecZlib::DoInflate(const char *input,
                                         size_t input_size,
                                         char *output,
                                         size_t *output_size) {
    int ret;

    if (!m_inflate_initialized) {
        memset(&m_stream_inflate, 0, sizeof(m_stream_inflate));
        m_stream_inflate.zalloc = Z_NULL;
        m_stream_inflate.zfree = Z_NULL;
        m_stream_inflate.opaque = Z_NULL;
        m_stream_inflate.avail_in = 0;
        m_stream_inflate.next_in = Z_NULL;

        ret = inflateInit(&m_stream_inflate);
        if (ret != Z_OK) {
            return COMPRESSION_E_UNPACK_ERROR;
        }

        m_inflate_initialized = true;
    }

    m_stream_inflate.avail_in = input_size;
    m_stream_inflate.next_in = (Bytef *)input;

    m_stream_inflate.avail_out = *output_size;
    m_stream_inflate.next_out = (Bytef *)output;

    ret = ::inflate(&m_stream_inflate, Z_NO_FLUSH);
    if (ret != Z_STREAM_END) {
        return COMPRESSION_E_UNPACK_ERROR;
    }

    if (m_stream_inflate.avail_out != 0) {
        return COMPRESSION_E_UNPACK_ERROR;
    }

    ::inflateReset(&m_stream_inflate);
    return COMPRESSION_E_OK;
}

}
