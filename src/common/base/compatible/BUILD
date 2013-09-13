cc_library(
    name = "compatible",
    srcs = [
        'errno.cpp',
        'io.cpp',
        'string.cpp',
        'time.cpp'
    ]
)

cc_test(
    name = "compatible_test",
    srcs = [
        "compatible_test.cpp"
    ],
    deps = ":compatible"
)

cc_test(
    name = "compatible_test_c",
    srcs = [
        "compatible_test.c"
    ],
    deps = ":compatible"
)

cc_test(
    name = "assert_test",
    srcs = "assert_test.cpp",
    deps = ":compatible"
)
