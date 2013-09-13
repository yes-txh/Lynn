# global variables
COMMON := ../../
THIRD_PARTY := ../../../thirdparty

# project settings
project.targets     := libini.a
project.includes    := $(COMMON)../ $(THIRD_PARTY)

# lib settings
libini.a.type       := library
libini.a.sources    := $(filter-out %test.cc, $(wildcard *.cc))

project.ldadd = \
	-pthread \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

project.targets += ini_file_test
project.tests += ini_file_test
ini_file_test.sources := ini_file_test.cc ini_file.cc

project.targets += file_ini_configer_test
project.tests += file_ini_configer_test
file_ini_configer_test.sources := \
	file_ini_configer_test.cc \
	file_ini_configer.cc \
	ini_file.cc \
	ini_configer.cc

file_ini_configer_test.ldadd := \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)..)/common/base/libbase.a

ini_file.cc.extra_flags := -Wno-format-nonliteral

# makefile framework
include ${COMMON}builder/Generic.mak
