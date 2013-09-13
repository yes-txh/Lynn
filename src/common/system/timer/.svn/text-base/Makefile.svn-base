root_path = ../../../..
output_path = $(root_path)/output/common/base

COMMON := ../..
THIRD_PARTY := ../../../thirdparty

project.includes = $(COMMON)/.. $(THIRD_PARTY)

project.targets :=

project.targets += timer_manager_test
project.tests += timer_manager_test
timer_manager_test.sources := timer_manager_test.cpp
timer_manager_test.ldadd = \
	$(if $(SOSO_BUILD),@libroot@/common/base/,../../base/)libbase.a \
	$(if $(SOSO_BUILD),@libroot@/common/system/,../)libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-lpthread

include $(COMMON)/builder/Generic.mak
