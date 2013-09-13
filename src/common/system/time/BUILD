cc_library(
    name = 'time',
    srcs = [
        'datetime.cpp',
        'posix_time.cpp',
        'stopwatch.cpp',
        'timestamp.cpp',
        'time_utils.cpp',
    ],
)

cc_test(
    name = 'datetime_test',
    srcs = 'datetime_test.cpp',
    deps = ':time'
)

cc_test(
    name = 'stopwatch_test',
    srcs = 'stopwatch_test.cpp',
    deps = [
        ':time',
        '//common/system/concurrency:concurrency'
    ]
)

cc_test(
    name = 'timestamp_test',
    srcs = 'timestamp_test.cpp',
    deps = [
        '#rt',
        ':time',
        '//common/system/concurrency:concurrency'
    ]
)
