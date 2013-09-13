COMMON := ../

project.debug    := 1
project.includes := $(COMMON)../ ../../thirdparty

project.targets += libbase_protocol.a
libbase_protocol.a.type := library
libbase_protocol.a.sources := base_protocol.cc

project.targets += libbase_protocol32.a
libbase_protocol32.a.type := library
libbase_protocol32.a.target_bits := 32
libbase_protocol32.a.sources := $(libbase_protocol.a.sources)

ifeq ($(HOSTTYPE),x86_64)
project.targets += libbase_protocol64.a
libbase_protocol64.a.type := library
libbase_protocol64.a.target_bits := 64
libbase_protocol64.a.sources := $(libbase_protocol.a.sources)
endif

project.targets += base_protocol_test
project.tests   += base_protocol_test

base_protocol_test.sources := ${COMMON}crypto/hash/crc.cpp base_protocol.cc base_protocol_test.cc
base_protocol_test.ldadd   := ../../thirdparty/gtest/lib@64@/libgtest.a -lpthread

#project.targets += encryption_test
#project.tests   += encryption_test

#encryption_test.sources := ${COMMON}crypto/hash/crc.cpp encryption_test.cc
#encryption_test.ldadd   := /usr/local/lib/libgtest.a -lpthread

include ${COMMON}builder/Generic.mak
