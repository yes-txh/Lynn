# Copyright 2011 Tencent Inc.
# Author: Yi Wang (yiwang@tencent.com)

proto_library(
    name = 'codex_proto',
    srcs = ['format_option.proto',
            'format_options.proto']
)

cc_library(
    name = 'codex',
    srcs = ['format_string_parser.cc',
            'proto_message_creator.cc'],
    deps = [':codex_proto',
            '//thirdparty/re2:re2',
            '//thirdparty/protobuf:protoc',
            '//thirdparty/protobuf:protobuf']
)

cc_test(
    name = 'format_string_parser_test',
    srcs = ['format_string_parser_test.cc'],
    deps = [':codex']
)

cc_test(
    name = 'proto_message_creator_test',
    srcs = ['proto_message_creator_test.cc'],
    deps = [':codex'],
    testdata = 'format_options.proto'
)
