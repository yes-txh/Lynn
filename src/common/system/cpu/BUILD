cc_library(
    name = 'cpu',
    srcs = [
        'cpu_usage.cpp'
    ],
    deps = '//common/base/string:string'
)

cc_test(
    name = 'cpu_usage_test',
    srcs = 'cpu_usage_test.cpp',
    deps = [
        ':cpu',
        '//thirdparty/gflags:gflags'
    ]
)

