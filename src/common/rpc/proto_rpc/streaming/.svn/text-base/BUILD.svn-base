
proto_library(
    name = "streaming_service_proto",
    srcs = "streaming_service.proto"
)

proto_library(
    name = "file_service_proto",
    srcs = "file_service.proto"
)

cc_library(
    name = "streaming",
    srcs = [
        "stream.cc",
        "stream_impl.cc",
        "stream_manager.cc",
        "streaming_service.cc",
    ],
    deps = [
        ":streaming_service_proto",
        "//common/rpc/proto_rpc:proto_rpc",
        "//common/base/string:string",
        "//common/netframe:netframe",
        "//common/system/timer:timer",
        "//thirdparty/glog:glog"
    ]
)

cc_binary(
    name = "test_client",
    srcs = [
        "client_test.cc"
    ],
    deps = [
        ":streaming",
        ":file_service_proto"
    ]
)

cc_binary(
    name = "test_server",
    srcs = [
        "server_test.cc"
    ],
    deps = [
        ":streaming",
        ":file_service_proto"
    ]
)
