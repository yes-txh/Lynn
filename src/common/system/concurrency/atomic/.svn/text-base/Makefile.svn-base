ROOT = ../../../../..
COMMON = ../../..
THIRD_PARTY = $(COMMON)/../thirdparty

project.extra_warning = -Wno-float-equal
project.optimize_flags = -O2
project.includes = $(COMMON)/.. $(THIRD_PARTY)

project.targets += atomic_test
project.tests += atomic_test
atomic_test.sources = atomic_test.cpp
atomic_test.ldadd = \
	-pthread \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

include $(COMMON)/builder/Generic.mak
