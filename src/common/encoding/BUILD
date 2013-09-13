cc_library(
    name = 'encoding',
    srcs = [
        'base64.cpp',
        'percent.cpp',
        'modp_b64/modp_b64.cpp',
    ]
)

cc_test(
    name = 'variant_integer_test',
    srcs = 'variant_integer_test.cpp'
)

cc_test(
    name = 'ascii_test',
    srcs = 'ascii_test.cpp'
)

cc_test(
    name = 'gb_test',
    srcs = 'gb_test.cpp'
)

cc_test(
    name = 'hex_test',
    srcs = 'hex_test.cpp'
)

cc_test(
    name = 'base64_test',
    srcs = 'base64_test.cpp',
    deps = ':encoding'
)

cc_test(
    name = 'percent_test',
    srcs = 'percent_test.cpp',
    deps = ':encoding'
)
