cc_library(
    name = 'file',
    srcs = 'file.cc',
    deps = [
        '//thirdparty/glog:glog',
        '//common/system:system'
    ]
)

cc_library(
    name = 'recordio',
    deps = '//common/file/recordio:recordio',
    deprecated = 1
)

cc_library(
    name = 'local_file',
    srcs = [
        'local_file.cc',
        "aioframe/aioframe.cc",
        "aioframe/work_thread.cc"
    ],
    deps = [
        ':file',
        '//common/system:system',
        '//common/system/concurrency:concurrency',
        '//common/text:wildcard',
        '//common/base/string:string',
        '//thirdparty/glog:glog'
    ],
    link_all_symbols = 1
)

cc_test(
    name = 'file_test',
    srcs = 'file_test.cc',
    deps = [
        ':local_file'
    ]
)

cc_test(
    name = "aio_test",
    srcs = [
        "aioframe/aio_test.cc",
    ],
    deps = [
        ":local_file",
        "//common/system/concurrency:concurrency",
        "//thirdparty/glog:glog"
    ]
)
