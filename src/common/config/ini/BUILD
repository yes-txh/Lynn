cc_library(
    name = 'ini',
    srcs = [
        'file_ini_configer.cc',
        'ini_configer.cc',
        'ini_file.cc',
        'memory_ini_configer.cc'
    ],
    deps = [
        '//common/base/string:string'
    ]
)

cc_test(
    name = 'ini_file_test',
    srcs = 'ini_file_test.cc',
    deps = ':ini'
)

cc_test(
    name = 'file_ini_configer_test',
    srcs = 'file_ini_configer_test.cc',
    deps = ':ini'
)

