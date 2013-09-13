/**
 * @file test.cpp
 * @brief
 * @author kypoyin
 * @date 2010-04-28
 */


#include <stdlib.h>
#include <stdio.h>
#include <gtest/gtest.h>

#include "common/compress/block_compression_codec_none.h"
#include "common/compress/block_compression_codec_zlib.h"
#include "common/compress/block_compression_codec_bmz.h"
#include "common/compress/block_compression_codec_lzo.h"
#include "common/compress/block_compression_codec_quicklz.h"
#include "common/compress/block_compression_codec_snappy.h"

using namespace intern;

int g_file_num = 0;
const char** g_file_array = NULL;

char* g_test_data = NULL;
size_t g_test_data_size = 0;

char* g_compressed_buf = NULL;
size_t g_compressed_buf_size = 0;

char* g_uncompressed_buf = NULL;
size_t g_uncompressed_buf_size = 0;

TEST(BlockCompressionCodec, InitTestData) {
    for(int i=0; i < g_file_num; i ++){
        const char *fname = g_file_array[i];
        ASSERT_TRUE(fname != NULL);

        FILE *fp = fopen(fname, "r+b");
        ASSERT_TRUE(fp != NULL);

        fseek(fp, 0, SEEK_END);

        int filesize = ftell(fp);
        g_test_data_size += filesize;

        fclose(fp);
    }

    g_test_data = (char *)malloc(g_test_data_size);
    ASSERT_TRUE(g_test_data != NULL);

    char  *p = g_test_data;
    for(int i=0; i < g_file_num; i++){
        const char *fname = g_file_array[i];
        ASSERT_TRUE(fname != NULL);

        FILE *fp = fopen(fname, "r+b");
        ASSERT_TRUE(fp != NULL);

        fseek(fp, 0, SEEK_END);

        int filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        fread(p, 1, filesize, fp);
        p += filesize;

        fclose(fp);
    }

    g_compressed_buf_size = g_test_data_size;
    g_compressed_buf = (char *)malloc(g_compressed_buf_size);

    g_uncompressed_buf_size = g_test_data_size;
    g_uncompressed_buf = (char *)malloc(g_uncompressed_buf_size);
}

bool CheckInflateAndDeflate(int type) {
    BlockCompressionCodec *bcc = NULL;

    switch (type) {
        case BlockCompressionCodec::NONE:
            bcc = new BlockCompressionCodecNone();
            break;

        case BlockCompressionCodec::ZLIB:
            bcc = new BlockCompressionCodecZlib();
            break;

        case BlockCompressionCodec::BMZ:
            bcc = new BlockCompressionCodecBmz();
            break;

        case BlockCompressionCodec::LZO:
            bcc = new BlockCompressionCodecLzo();
            break;

        case BlockCompressionCodec::QUICKLZ:
            bcc = new BlockCompressionCodecQuicklz();
            break;

        case BlockCompressionCodec::SNAPPY:
            bcc = new BlockCompressionCodecSnappy();
            break;

        default:
            printf("Invalid Type [%d]\n!", type);
            return false;
    }


    bcc->Deflate(g_test_data, g_test_data_size,
                 g_compressed_buf, &g_compressed_buf_size);
    bcc->Inflate(g_compressed_buf, g_compressed_buf_size,
                 g_uncompressed_buf, &g_uncompressed_buf_size);

    int ret = memcmp(g_uncompressed_buf, g_test_data, g_test_data_size);
    EXPECT_EQ(0, ret);

    return (ret == 0);
}


TEST(BlockCompressionCodec, Codec) {
    CheckInflateAndDeflate(BlockCompressionCodec::NONE);
    CheckInflateAndDeflate(BlockCompressionCodec::ZLIB);
    CheckInflateAndDeflate(BlockCompressionCodec::BMZ);
    CheckInflateAndDeflate(BlockCompressionCodec::LZO);
    CheckInflateAndDeflate(BlockCompressionCodec::QUICKLZ);
    CheckInflateAndDeflate(BlockCompressionCodec::SNAPPY);
}

int main (int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    const char *defaultfile= "./data/000080.htm";

    if (argc > 1) {
        g_file_num = argc - 1;
        g_file_array = (const char **)(&argv[1]);
    } else {
        g_file_num = 1;
        g_file_array = &defaultfile;
    }

    return RUN_ALL_TESTS();
}
