COMMON := ../..
THIRD_PARTY := ../../../thirdparty

project.includes = $(COMMON)/.. $(THIRD_PARTY)
project.ldadd := \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

project.targets += intrusive_list_test
project.tests += intrusive_list_test
intrusive_list_test.sources := intrusive_list_test.cpp

project.targets += algorithm_test
project.tests += algorithm_test
algorithm_test.sources = algorithm_test.cpp

project.targets += array_test
project.tests += array_test
array_test.sources := array_test.cpp

project.targets += pointee_compare_test
project.tests += pointee_compare_test
pointee_compare_test.sources := pointee_compare_test.cpp

project.targets += shared_ptr_test
project.tests += shared_ptr_test
shared_ptr_test.sources := shared_ptr_test.cpp

include $(COMMON)/builder/Generic.mak

