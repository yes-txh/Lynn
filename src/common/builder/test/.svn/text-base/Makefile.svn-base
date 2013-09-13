builder.verbose =

project.includes = include
project.defines = SSSSSSSSSSSSSSSS
project.subdirs = . build
project.target_bits := 32

project.targets += libso.so
libso.so.type = shared
libso.so.sources = src/so.c

project.targets += liba.a
liba.a.type = library
liba.a.sources = src/so.c
liba.a.extra_flags = -fPIC
liba.a.path := lib1

project.targets += a
a.sources = src/a/main.cpp
a.includes = ../
a.defines = UUUU
a.ldadd = -lz @libroot@/libso.so /usr/lib@64@/libc.a

src/a/main.cpp.extra_flags = -O2

project.targets += b
b.sources = src/b/main.cpp
b.includes = src/b

project.targets += c
c.sources = src/c/main.c
c.test_args = 1 2
c.ldadd = -lz @libroot@/libso.so /usr/lib@64@/libc.a

project.tests = b c

include Generic.mak
