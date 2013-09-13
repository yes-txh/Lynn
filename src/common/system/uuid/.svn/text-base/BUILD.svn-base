cc_library(
    name = "uuid",
    srcs = [
        "clear.c",
        "copy.c",
        "gen_uuid_nt.c",
        "pack.c",
        "unparse.c",
        "compare.c",
        "gen_uuid.c",
        "isnull.c",
        "parse.c",
        "unpack.c",
        "uuid_time.c"
    ],
    extra_cppflags = ["-Wno-format-nonliteral"]
)

cc_test(
    name = "uuid_test",
    srcs = "uuid_test.cpp",
    deps = ":uuid"
)

