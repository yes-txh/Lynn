/**
 * @file BlockCompressionCodecBmz.cc
 * @brief
 * @author kypoyin
 * @date 2010-04-29
 */

#include <string.h>
#include <assert.h>
#include "common/compress/bmz/bmz.h"
#include "common/compress/block_compression_codec_bmz.h"

using namespace std;

namespace intern {

BlockCompressionCodecBmz::BlockCompressionCodecBmz()
    : m_workmem(NULL), m_workmem_size(0), m_fp_len(5) {
    assert(bmz_init() == BMZ_E_OK);
}

BlockCompressionCodecBmz::~BlockCompressionCodecBmz() {
    if (m_workmem) {
        free((void *)m_workmem);
    }

    m_workmem_size = 0;
}

void BlockCompressionCodecBmz::SetArgs(const char *name, const char *value) {
    if (0 == strcasecmp(name, "fp-len")) {
        m_fp_len = atoi(value);
    }
}

int BlockCompressionCodecBmz::DoDeflate(const char *input,
                                        size_t input_size,
                                        char *output,
                                        size_t *output_size) {
    size_t inlen = input_size;

    // compute the size of auxiliary buffer
    size_t worklen = bmz_pack_worklen(inlen, m_fp_len);

    if (m_workmem_size < worklen) {
        if (m_workmem) {
            free((void *)m_workmem);
        }

        m_workmem = (char *)malloc(worklen);
        m_workmem_size = worklen;
    }

    if (bmz_pack(input, inlen, output, output_size, m_fp_len, m_workmem) !=
        BMZ_E_OK) {
        return COMPRESSION_E_PACK_ERROR;
    }

    return COMPRESSION_E_OK;
}

int BlockCompressionCodecBmz::DoInflate(const char *input,
                                        size_t input_size,
                                        char *output,
                                        size_t *output_size) {
    // ����ڴ����Ԥ������ڴ治���������·���
    size_t worklen = bmz_unpack_worklen(*output_size);
    if (worklen > m_workmem_size) {
        if (m_workmem) {
            free((void *)m_workmem);
        }

        m_workmem = (char *)malloc(worklen);
        m_workmem_size = worklen;
    }

    if (bmz_unpack(input, input_size, output, output_size, m_workmem) !=
        BMZ_E_OK) {
        return COMPRESSION_E_UNPACK_LEN_ERROR;
    }

    return COMPRESSION_E_OK;
}

}
