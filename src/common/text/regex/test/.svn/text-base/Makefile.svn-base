COMMON=../../..

project.c_includes = ../pcre
project.cxx_includes = ..
project.includes = $(COMMON)/..
project.defines = HAVE_CONFIG_H
project.ldadd = $(if $(SOSO_BUILD),@libroot@/common/text/regex/,../)libregex.a

project.targets += pcretest
pcretest.sources = pcretest.c
pcretest.ldadd := -lz

project.targets += pcredemo
pcredemo.sources := pcredemo.c

project.targets += pcregrep
pcregrep.sources := pcregrep.c
pcregrep.ldadd := -lz -lbz2

project.targets += regex_unittest
regex_unittest.sources := regex_unittest.cpp

project.targets += regex_scanner_unittest
regex_scanner_unittest.sources := regex_scanner_unittest.cpp

project.extra_clean := clean_test_output

include $(COMMON)/builder/Generic.mak

clean_test_output:
	@$(RM) testNinput  testsavedregex  teststderr  testtry
