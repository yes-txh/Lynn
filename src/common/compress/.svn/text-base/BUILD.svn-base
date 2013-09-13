cc_library(
    name = 'compress',
    srcs = [
        'bmz/bmdiff.cc',
        'bmz/bmz.cc',
        'checksum/checksum.cc',
        'lzo/minilzo.c',
        'quicklz/quicklz.cc',
        'block_compression_codec_bmz.cc',
        'block_compression_codec.cc',
        'block_compression_codec_lzo.cc',
        'block_compression_codec_manager.cc',
        'block_compression_codec_none.cc',
        'block_compression_codec_quicklz.cc',
        'block_compression_codec_snappy.cc',
        'block_compression_codec_zlib.cc',
    ],
    deps = [
        '//thirdparty/snappy:snappy',
        '//common/system/concurrency:concurrency',
        '#z'
    ]
)

cc_test(
    name = 'block_compression_codec_test',
    srcs = 'block_compression_codec_test.cc',
    deps = ':compress',
    testdata = 'data/000080.htm'
)

