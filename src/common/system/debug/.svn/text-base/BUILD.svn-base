cc_test(
    name = 'print_trace_test',
    srcs = ['print_trace_test.cc']
)

cc_library(
    name = 'debug',
    srcs = 'hex_dump.cpp'
)

cc_test(
    name = 'hex_dump_test',
    srcs = ['hex_dump_test.cpp'],
    deps = ':debug'
)

