ifndef SOSOBUILD
COMMON := ..
THIRD_PARTY := ../../thirdparty
project.predefined_includes := $(COMMON)/.. $(THIRD_PARTY)
endif

project.targets += bloom_filter_test
project.tests += bloom_filter_test

bloom_filter_test.sources = bloom_filter_test.cpp
bloom_filter_test.ldadd = \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/crypto/libcrypto.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	$(THIRD_PARTY)/openssl/lib@64@/libcrypto.a \
	-pthread

include ../builder/Generic.mak
