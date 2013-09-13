# global variables
COMMON := ../../
THIRD_PARTY = ../../../thirdparty

# project settings
project.includes    := $(COMMON)../ $(THIRD_PARTY)

# lib settings
project.targets     += xml_configer_test
xml_configer_test.sources := xml_configer_test.cpp xml_configer.cpp
xml_configer_test.ldadd := \
	-pthread \
	$(COMMON)/base/libbase.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

# makefile framework
include ${COMMON}builder/Generic.mak
