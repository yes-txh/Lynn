cc_library(
    name = 'ref_counted',
    srcs = 'ref_counted.cpp'
)

cc_test(
    name = 'ref_counted_test',
    srcs = 'ref_counted_test.cpp',
    deps = ':ref_counted'
)

cc_library(
    name = 'function',
    srcs = [
        'function/function_base.cpp'
    ],
    deps = ':ref_counted'
)

cc_test(
    name = 'function_test',
    srcs = [
        'function_test.cpp'
    ],
    deps = [
        '//common/base:function',
        '//thirdparty/gmock:gmock'
    ]
)

cc_test(
    name = 'any_ptr_test',
    srcs = ['any_ptr_test.cpp']
)

cc_test(
    name = 'array_size_test',
    srcs = ['array_size_test.cpp']
)

cc_test(
    name = 'closure_test',
    srcs = ['closure_test.cpp']
)

cc_test(
    name = 'object_pool_test',
    srcs = ['object_pool_test.cpp'],
    deps = '//common/system/concurrency:concurrency'
)

cc_test(
    name = 'byte_order_test',
    srcs = [
        'byte_order_test.cpp'
    ],
)

cc_test(
    name = 'global_initialize_test',
    srcs = 'global_initialize_test.cpp'
)

cc_test(
    name = 'int_cast_test',
    srcs = 'int_cast_test.cpp'
)


cc_test(
    name = 'class_register_test',
    srcs = [
        'class_register_test.cc',
        'class_register_test_helper.cc'
    ]
)

cc_test(
    name = 'global_function_register_test',
    srcs = [
        'global_function_register_test.cc'
    ]
)

cc_test(
    name = 'stdint_test',
    srcs = [
        'stdint_test.cpp'
    ]
)

cc_test(
    name = 'uint32_divisor_test',
    srcs = [
        'uint32_divisor_test.cpp'
    ]
)

cc_test(
    name = 'singleton_test',
    srcs = 'singleton_test.cpp'
)

cc_test(
    name = 'binary_version_test',
    srcs = [
        'binary_version_test.cpp',
    ]
)

cc_test(
    name = 'export_variable_test',
    srcs = 'export_variable_test.cc',
    deps = [
        '//common/system:system',
        '//common/system/concurrency:concurrency'
    ]
)

cc_library(
    name = 'module',
    srcs = [
        'module/module.cpp',
        'module/dependency_map.cpp'
    ],
    deps = [
        '//thirdparty/glog:glog',
        '//thirdparty/gtest:gtest'
    ]
)

cc_test(
    name = "timed_stats_test",
    srcs = "timed_stats_test.cc",
    deps = [
        '//common/system/concurrency:concurrency',
        '//common/system/time:time'
    ]
)

cc_test(
    name = 'strict_bool_test',
    srcs = 'strict_bool_test.cpp'
)

cc_test(
    name = 'container_of_test',
    srcs = 'container_of_test.cpp'
)
