ifndef SOSOBUILD
ROOT = ../../..
COMMON = ..
THIRD_PARTY = ../../thirdparty
project.includes := $(COMMON)/.. $(THIRD_PARTY)
endif

project.targets := libcrypto.a
project.subdirs = random hash

libcrypto.a.type := library
libcrypto.a.sources := \
	$(filter-out %test.cpp, \
		$(wildcard \
			*.cpp \
			random/*.cpp \
			hash/*.cpp \
			hash/murmur/*.cpp \
			hash/fnv/*.c \
			hash/tiger/*.c \
		) \
	)

project.targets += libcrypto32.a
libcrypto32.a.type := library
libcrypto32.a.target_bits := 32
libcrypto32.a.sources := $(libcrypto.a.sources)

ifeq ($(HOSTTYPE),x86_64)
project.targets += libcrypto64.a
libcrypto64.a.type := library
libcrypto64.a.target_bits := 64
libcrypto64.a.sources := $(libcrypto.a.sources)

project.targets += libcrypto64.so
libcrypto64.so.type := shared
libcrypto64.so.target_bits := 64
libcrypto64.so.sources := $(libcrypto.a.sources)
libcrypto64.so.ldadd := $(THIRD_PARTY)/openssl/lib@64@/libcrypto.so
endif

include $(COMMON)/builder/Generic.mak
