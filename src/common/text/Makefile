COMMON = ..
THIRD_PARTY = ../../thirdparty

project.subdirs = regex

project.includes := $(COMMON)/.. $(THIRD_PARTY)

project.targets := libwildcard.a
libwildcard.a.type := library
libwildcard.a.sources := wildcard.cpp

project.targets += libwildcard32.a
libwildcard32.a.type := library
libwildcard32.a.target_bits := 32
libwildcard32.a.sources := wildcard.cpp

project.targets += libwildcard.so
libwildcard.so.type := shared
libwildcard.so.sources := wildcard.cpp

project.targets += libwildcard32.so
libwildcard32.so.type := shared
libwildcard32.so.target_bits := 32
libwildcard32.so.sources := wildcard.cpp

ifeq ($(HOSTTYPE),x86_64)
project.targets += libwildcard64.so
libwildcard64.so.type := shared
libwildcard64.so.target_bits := 64
libwildcard64.so.sources := wildcard.cpp
endif

project.targets += wildcard_test
wildcard_test.sources = wildcard.cpp wildcard_test.cpp
wildcard_test.ldadd = $(THIRD_PARTY)/gtest/lib@64@/libgtest.a -pthread \
                      $(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

include $(COMMON)/builder/Generic.mak
