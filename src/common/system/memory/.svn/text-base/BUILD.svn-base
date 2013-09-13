cc_library(
    name = 'memory',
    srcs = [
        'mempool.cpp',
        'mem_usage.cpp'
    ],
    deps = "//common/system/concurrency:concurrency"
)

cc_test(
    name = 'mempool_test',
    srcs = 'mempool_test.cpp',
    deps = ':memory'
)

cc_test(
    name = 'barrier_test',
    srcs = 'barrier_test.cpp',
    deps = ':memory'
)

cc_test(
    name = 'unaligned_test',
    srcs = 'unaligned_test.cpp'
)

cc_test(
    name = 'mem_usage_test',
    srcs = 'mem_usage_test.cpp',
    deps = ':memory'
)

