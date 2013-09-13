ifndef SOSOBUILD
ROOT = ../../../..
COMMON = ../..
THIRD_PARTY = $(COMMON)/../thirdparty
project.includes = $(COMMON)/.. $(THIRD_PARTY)
endif

project.ldadd := \
	-pthread \
	$(if $(SOSO_BUILD),@libroot@/common/system/,../)libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

project.targets += mempool_test
project.tests += mempool_test
mempool_test.sources := mempool_test.cpp mempool.cpp

project.targets += unaligned_test
project.tests += unaligned_test
unaligned_test.sources := unaligned_test.cpp

project.targets += mem_usage_test
mem_usage_test.sources = mem_usage_test.cpp
mem_usage_test.ldadd += $(COMMON)/system/libsystem.a

include $(COMMON)/builder/Generic.mak

