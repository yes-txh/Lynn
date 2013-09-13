cc_library(
    name = 'io',
    srcs = [
        'directory.cpp',
        'file.cpp',
        'textfile.cpp',
        'path.cpp',
        'file_utility.cpp'
    ],
    deps = [
        '//common/text:wildcard',
        '//common/base/string:string'
    ]
)

cc_test(
    name = 'path_test',
    srcs = 'path_test.cpp',
    deps = ':io'
)

cc_test(
    name = 'file_test',
    srcs = 'file_test.cpp',
    deps = ':io'
)

cc_test(
    name = 'textfile_test',
    srcs = 'textfile_test.cpp',
    deps = ':io',
    testdata = ['test_dos.txt', 'test_unix.txt', 'xxxx']
)

cc_test(
    name = 'directory_test',
    srcs = 'directory_test.cpp',
    deps = ':io'
)
