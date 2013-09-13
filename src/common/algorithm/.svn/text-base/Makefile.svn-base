COMMON = ..
THIRD_PARTY = ../../thirdparty

project.includes = $(COMMON)/.. $(THIRD_PARTY)

project.targets += integer_test
project.tests += integer_test
integer_test.sources := integer_test.cpp
integer_test.ldadd := \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

include $(COMMON)/builder/Generic.mak
