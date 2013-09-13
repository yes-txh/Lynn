ifndef SOSOBUILD
COMMON := ../..
THIRD_PARTY := ../../../thirdparty
project.predefined_includes := $(COMMON)/.. $(THIRD_PARTY)
endif

project.ldadd  := \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	$(THIRD_PARTY)/openssl/lib@64@/libcrypto.a \
	-lpthread
project.includes := $(COMMON)/..

project.targets += crc_test
project.tests   += crc_test
crc_test.sources := crc.cpp crc_test.cc

project.targets += md5_test
project.tests += md5_test
md5_test.sources = md5_test.cpp md5.cpp

project.targets += performance_test
project.tests += performance_test
performance_test.sources := \
	crc.cpp \
	performance_test.cpp \
	md5.cpp \
	md4.cpp \
	murmur/MurmurHash2.cpp \
	murmur/MurmurHash3.cpp \
	city.cpp \
	fingerprint.cpp

project.targets += sha1_test
project.tests += sha1_test
sha1_test.sources = sha1_test.cpp

include $(COMMON)/builder/Generic.mak
