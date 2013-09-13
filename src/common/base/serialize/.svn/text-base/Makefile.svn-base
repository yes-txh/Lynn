COMMON := ../..
ROOT := $(COMMON)/../..

project.includes := $(COMMON)/.. $(COMMON)/../thirdparty

project.targets += serialize_test
project.tests += serialize_test
serialize_test.sources := serialize_test.cpp
serialize_test.ldadd := \
	$(COMMON)/../thirdparty/gtest/lib@64@/libgtest.a \
	$(COMMON)/../thirdparty/gtest/lib@64@/libgtest_main.a \
	-pthread

# this source has many float-equal test, disable float-equal waring
project.extra_warning = -Wno-float-equal

include $(COMMON)/builder/Generic.mak

