cc_library(
    name = 'hash',
    srcs = [
        'crc.cpp',
        'md4.cpp',
        'md5.cpp',
        'string_hashes.cpp',
        'city.cpp',
        'fingerprint.cpp'
    ],
    deps = [
        '//thirdparty/openssl:crypto',
        '//common/crypto/hash/murmur:murmur',
        '//common/crypto/hash/fnv:fnv'
    ]
)

cc_test(
    name = 'crc_test',
    srcs = 'crc_test.cc',
    deps = ':hash'
)

cc_test(
    name = 'md5_test',
    srcs = 'md5_test.cpp',
    deps = ':hash'
)

cc_test(
    name = 'sha1_test',
    srcs = 'sha1_test.cpp',
    deps = ':hash'
)

cc_test(
    name = 'performance_test',
    srcs = 'performance_test.cpp',
    deps = ':hash'
)

