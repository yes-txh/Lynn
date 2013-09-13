COMMON := ../..
THIRD_PARTY := ../../../thirdparty

project.includes = $(COMMON)/.. $(THIRD_PARTY)
project.targets += compatible_test
compatible_test.sources = compatible_test.cpp
compatible_test.ldadd := \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

include $(COMMON)/builder/Generic.mak
