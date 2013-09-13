
ROOT = ../../../..
COMMON = ../..
THIRD_PARTY = $(COMMON)/../thirdparty

project.includes = $(COMMON)/.. $(THIRD_PARTY)

project.targets += socket_test
socket_test.sources += socket_test.cpp
socket_test.ldadd = \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

project.targets += get_ip_test
get_ip_test.sources += get_ip_test.cpp
get_ip_test.ldadd = \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

include ../../builder/Generic.mak
