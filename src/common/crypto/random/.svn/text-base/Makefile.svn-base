ifndef SOSOBUILD
ROOT = ../../../..
COMMON = ../..
THIRD_PARTY = ../../../thirdparty
project.includes := $(COMMON)/.. $(THIRD_PARTY)
endif

project.targets += true_random_test

project.tests += true_random_test
true_random_test.sources = true_random_test.cpp true_random.cpp
true_random_test.ldadd = \
	-pthread \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

project.targets += pseudo_random_test
project.tests += pseudo_random_test
pseudo_random_test.sources = pseudo_random_test.cpp pseudo_random.cpp
pseudo_random_test.ldadd = \
	-pthread \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \

include $(COMMON)/builder/Generic.mak
