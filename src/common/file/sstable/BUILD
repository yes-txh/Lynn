proto_library(
    name = 'sstable_header',
    srcs = 'sstable_header.proto'
)

proto_library(
    name = 'sstable_block_header',
    srcs = 'sstable_block_header.proto'
)

cc_library(
    name = 'sstable',
    srcs = [
        'block_cache.cc',
        'multi_sstables_reader.cc',
        'sstable_def.cc',
        'sstable_merger.cc',
        'sstable_reader_block.cc',
        'sstable_reader.cc',
        'sstable_writer_block.cc',
        'sstable_writer.cc',
        'test_helper.cc',
    ],
    deps = [
        ':sstable_header',
        ':sstable_block_header',
        '//common/compress:compress',
        '//common/crypto/hash:hash',
        '//common/file:file',
        '//common/file:local_file',
        '//common/system/time:time',
        '//thirdparty/glog:glog'
    ]
)

cc_test(
    name = 'sstable_reader_block_test',
    srcs = 'sstable_reader_block_test.cc',
    deps = [
        ':sstable',
        '//common/file:local_file',
    ],
    testdata = 'url.dat'
)

cc_test(
    name = 'sstable_writer_block_test',
    srcs = 'sstable_writer_block_test.cc',
    deps = [
        ':sstable',
        '//common/file:local_file',
    ],
    testdata = 'url.dat'
)

cc_test(
    name = 'sstable_reader_test',
    srcs = 'sstable_reader_test.cc',
    deps = [
        ':sstable',
        '//common/file:local_file',
    ],
    testdata = 'url.dat'
)

cc_test(
    name = 'sstable_writer_test',
    srcs = 'sstable_writer_test.cc',
    deps = [
        ':sstable',
        '//common/file:local_file',
    ],
    testdata = 'url.dat'
)

cc_test(
    name = 'block_cache_test',
    srcs = 'block_cache_test.cc',
    deps = [
        ':sstable',
        '//common/file:local_file',
    ],
    testdata = 'url.dat'
)

cc_test(
    name = 'sstable_reader_iterator_test',
    srcs = 'sstable_reader_iterator_test.cc',
    deps = [
        ':sstable',
        '//common/file:local_file',
    ],
    testdata = 'url.dat'
)

cc_test(
    name = 'multi_sstables_reader_test',
    srcs = 'multi_sstables_reader_test.cc',
    deps = [
        ':sstable',
        '//common/file:local_file',
        '//common/base/string:string'
    ],
    testdata = 'url.dat'
)

cc_test(
    name = 'multi_readers_iterator_test',
    srcs = 'multi_readers_iterator_test.cc',
    deps = [
        ':sstable',
        '//common/file:local_file',
        '//common/base/string:string'
    ],
    testdata = 'url.dat'
)

cc_test(
    name = 'sstable_merger_test',
    srcs = 'sstable_merger_test.cc',
    deps = [
        ':sstable',
        '//common/file:local_file',
        '//common/base/string:string'
    ],
    testdata = 'url.dat'
)

cc_test(
    name = 'sstable_def_test',
    srcs = 'sstable_def_test.cc',
    deps = [
        ':sstable',
        '//common/file:local_file'
    ]
)

