project.includes = ../../../../../

project.targets :=

project.targets += make_test
make_test.sources := $(wildcard *.cpp)
make_test.ldadd := ../../../../system/libsystem.a
make_test.force_ldadd = ../../../../base/libbase@64@.a

include ../../../Generic.mak
