ROOT = ../../..
COMMON = ../../common
THIRD_PARTY := ../../thirdparty
project.debug = 1
project.defines = USE_MD5
project.includes := $(COMMON)/.. $(THIRD_PARTY)

project.targets += libweb.a
libweb.a.type := library
libweb.a.sources := \
	$(filter-out %_test.cc %_test.cpp, \
		$(wildcard *.cc) \
		$(wildcard *.cpp) \
	)

project.targets += libweb.so
libweb.so.type := shared
libweb.so.sources := $(libweb.a.sources)
libweb.so.ldadd := \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/crypto/libcrypto.a \
	$(THIRD_PARTY)/openssl/lib@64@/libcrypto.a \

project.targets += domain_test
project.tests += domain_test
domain_test.sources := domain_test.cc
domain_test.ldadd := \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/web/libweb.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/crypto/libcrypto.a \
	$(THIRD_PARTY)/openssl/lib@64@/libcrypto.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

project.targets += url_test
project.tests += url_test
url_test.sources := url_test.cpp
url_test.ldadd := \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/web/libweb.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/crypto/libcrypto.a \
	$(THIRD_PARTY)/openssl/lib@64@/libcrypto.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread 

include $(COMMON)/builder/Generic.mak
