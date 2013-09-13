ROOT := ../../../..
COMMON := ../..
THIRD_PARTY := ../../../thirdparty

PROTOC := $(THIRD_PARTY)/protobuf/bin/protoc

project.prebuild := recordio_test.pb.h recordio_test.pb.cc recordio_extent_header.pb.h recordio_extent_header.pb.cc
project.extra_clean_files := $(project.prebuild)
project.generated_files := $(project.prebuild)

project.targets += librecordio_32.a
librecordio_32.a.type := library
librecordio_32.a.target_bits := 32
librecordio_32.a.sources := recordio.cc recordio_jna.cc recordio_extent_header.pb.cc

project.targets += librecordio_32d.a
librecordio_32d.a.type := library
librecordio_32d.a.debug := y
librecordio_32d.a.target_bits := 32
librecordio_32d.a.sources := $(librecordio_32.a.sources)

ifeq ($(HOSTTYPE),x86_64)
	project.targets += librecordio_64.a
	librecordio_64.a.type := library
	librecordio_64.a.target_bits := 64
	librecordio_64.a.sources := $(librecordio_32.a.sources)

project.targets += librecordio_64d.a
librecordio_64d.a.type := library
librecordio_64d.a.debug := y
librecordio_64d.a.target_bits := 64
librecordio_64d.a.sources := $(librecordio_32.a.sources)
endif

#project.targets += recordio_test
project.tests += recordio_test
recordio_test.sources := recordio_test.cc recordio_test.pb.cc
recordio_test.target_bits := 64
recordio_test.ldadd = -pthread \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/compress/libcompress.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/file/librecordio_64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/file/libnewfile_64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/text/libwildcard.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/crypto/libcrypto.a \
	$(COMMON)/compress/libcompress.a \
	$(THIRD_PARTY)/protobuf/lib@64@/libprotobuf.a \
	$(THIRD_PARTY)/protobuf/lib@64@/libprotobuf-lite.a \
	$(THIRD_PARTY)/gflags/lib@64@/libgflags.a \
	$(THIRD_PARTY)/glog/lib@64@/libglog.a \
	$(THIRD_PARTY)/gflags/lib@64@/libgflags.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	$(ROOT)/src/thirdparty/snappy/lib@target_bits@_release/libsnappy.a\
	-lz

recordio_test.force_ldadd := $(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/file/liblocalfile_64.a

include $(COMMON)/builder/Generic.mak

recordio_test.pb.h recordio_test.pb.cc : recordio_test.proto
	$(PROTOC) --cpp_out=. recordio_test.proto

recordio_extent_header.pb.h recordio_extent_header.pb.cc : recordio_extent_header.proto
	$(PROTOC) --cpp_out=. recordio_extent_header.proto


