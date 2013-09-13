proto_library(
    name = 'recordio_extent_header_proto',
    srcs = 'recordio_extent_header.proto'
)

cc_library(
    name = 'recordio',
    srcs = [
        'recordio.cc',
        'recordio_jna.cc'
    ],
    deps = [
        ':recordio_extent_header_proto',
        '//common/compress:compress',
        '//common/crypto/hash:hash',
        '//common/crypto/random:random',
        '//thirdparty/glog:glog',
        '//thirdparty/protobuf:protobuf',
        '//common/crypto/hash:hash',
        '//common/crypto/random:random',
        '//common/file:file'
    ]
)

proto_library(
    name = 'recordio_test_proto',
    srcs = 'recordio_test.proto'
)

cc_binary(
    name = 'recordio_jna_test',
    srcs = 'recordio_jna_test.cc',
    deps = ':recordio'
)

cc_binary(
    name = 'recordio_dump',
    srcs = 'recordio_dump.cc',
    deps = [
        ':recordio',
        '//common/file:local_file',
        '//common/system/debug:debug'
    ],
)

cc_test(
    name = 'recordio_test',
    srcs = 'recordio_test.cc',
    deps = [
        ':recordio',
        ':recordio_test_proto',
        '//common/file:local_file'
    ],
    testdata = 'master_file_list_old'
)

