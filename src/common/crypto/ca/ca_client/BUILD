cc_library(
    name = "ca_client",
    srcs = "certification.cc",
    deps = [
        "//common/baselib/svrpublib:svrpublib",
        "//common/encoding:encoding",
        "//common/base/string:string",
        "//common/base:module",
        "//common/crypto/hash:hash"
    ]
)

cc_test(
    name = "certification_test",
    srcs = [
        "certification_test.cc",
        "main_unittest.cc"
    ],
    deps = [
        ":ca_client",
        "//thirdparty/glog:glog"
    ],
    testdata = [
        "fatliu_private_key.dat",
        "pecywang_private_key.dat"
    ]
)
