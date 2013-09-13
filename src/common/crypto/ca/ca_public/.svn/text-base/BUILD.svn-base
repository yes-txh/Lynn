proto_library(
    name = "ca_proto",
    srcs = [
        "ca_rpc.proto"
    ],
    deps = [
        # use rpc_option
       "//common/rpc/proto_rpc:proto_rpc"
    ]
)

cc_test(
    name = "ca_error_code_test",
    srcs = [
        "ca_error_code_test.cc",
    ],
	deps = [
        "//common/baselib/svrpublib:svrpublib"
    ]
)
