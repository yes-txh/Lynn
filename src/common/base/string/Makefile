COMMON := ../..
THIRD_PARTY := ../../../thirdparty

project.includes = $(COMMON)/.. $(THIRD_PARTY)
project.extra_warning = -Wno-float-equal
project.ldadd = \
	$(if $(SOSO_BUILD),@libroot@/common/base/,../)libbase.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	-lpthread

project.targets += algorithm_test
project.tests += algorithm_test
algorithm_test.sources := algorithm_test.cpp

project.targets += string_number_test
project.tests += string_number_test
string_number_test.sources := string_number_test.cpp

project.targets += string_piece_test
project.tests += string_piece_test
string_piece_test.sources := string_piece_test.cpp

include $(COMMON)/builder/Generic.mak
