cc_library(
    name = 'generator',
    srcs = 'file_generator.cpp',
    deps = '//common/base/string:string'
)

lex_yacc_library(
    name = 'parser',
    srcs = ['parser.ll', 'parser.yy'],
    deps = ['//common/base/string:string', ':generator']
)

cc_binary(
    name = 'rpcc',
    srcs = [
        'idlcompiler.cpp',
    ],
    deps = [
        ':parser',
    ]
)

