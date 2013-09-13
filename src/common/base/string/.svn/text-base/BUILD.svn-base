cc_library(
    name = "string",
    srcs = [
        'algorithm.cpp',
        'string_number.cpp',
        'string_piece.cpp',
        'format.cpp',
        'concat.cpp'
    ],
)

cc_test(
    name = "string_piece_test",
    srcs = ['string_piece_test.cpp'],
    deps = [':string']
)

cc_test(
    name = "algorithm_test",
    srcs = ['algorithm_test.cpp'],
    deps = [':string']
)

cc_test(
    name = "string_number_test",
    srcs = ['string_number_test.cpp'],
    deps = [':string'],
    extra_cppflags = "-Wno-float-equal"
)

cc_test(
    name = "format_test",
    srcs = ['format_test.cpp'],
    deps = [':string']
)

proto_library(
    name = "concat_test_proto",
    srcs = "concat_test.proto"
)

cc_test(
    name = "concat_test",
    srcs = ['concat_test.cpp'],
    deps = [':string', ':concat_test_proto', '//thirdparty/perftools:profiler']
)

cc_test(
    name = "byte_set_test",
    srcs = ['byte_set_test.cpp'],
    deps = [':string']
)

