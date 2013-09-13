ROOT := ../../..
COMMON := ..
THIRD_PARTY := ../../thirdparty

PROTOC := $(THIRD_PARTY)/protobuf/bin/protoc

project.subdirs = . recordio sstable
project.postbuild = file_build_finished

project.targets += libnewfile_32.a
libnewfile_32.a.type := library
libnewfile_32.a.target_bits := 32
libnewfile_32.a.sources := file.cc

project.targets += libnewfile_32d.a
libnewfile_32d.a.type := library
libnewfile_32d.a.debug := y
libnewfile_32d.a.target_bits := 32
libnewfile_32d.a.sources := $(libnewfile_32.a.sources)

ifeq ($(HOSTTYPE),x86_64)
project.targets += libnewfile_64.a
libnewfile_64.a.type := library
libnewfile_64.a.target_bits := 64
libnewfile_64.a.sources := $(libnewfile_32.a.sources)

project.targets += libnewfile_64d.a
libnewfile_64d.a.type := library
libnewfile_64d.a.debug := y
libnewfile_64d.a.target_bits := 64
libnewfile_64d.a.sources := $(libnewfile_32.a.sources)
endif

project.targets += libnewfile_32.so
libnewfile_32.so.type := shared
libnewfile_32.so.target_bits := 32
libnewfile_32.so.sources := $(libnewfile_32.a.sources)
libnewfile_32.so.ldadd = -pthread \
                      $(THIRD_PARTY)/glog/lib@64@/libglog.so

project.targets += libnewfile_32d.so
libnewfile_32d.so.type := shared
libnewfile_32d.so.debug := y
libnewfile_32d.so.target_bits := 32
libnewfile_32d.so.sources := $(libnewfile_32.a.sources)
libnewfile_32d.so.ldadd = -pthread \
	$(THIRD_PARTY)/glog/lib@64@/libglog.so

ifeq ($(HOSTTYPE),x86_64)
project.targets += libnewfile_64.so
libnewfile_64.so.type := shared
libnewfile_64.so.target_bits := 64
libnewfile_64.so.sources := $(libnewfile_32.a.sources)
libnewfile_64.so.ldadd = -pthread \
	$(THIRD_PARTY)/glog/lib@64@/libglog.so

project.targets += libnewfile_64d.so
libnewfile_64d.so.type := shared
libnewfile_64d.so.debug := y
libnewfile_64d.so.target_bits := 64
libnewfile_64d.so.sources := $(libnewfile_32.a.sources)
libnewfile_64d.so.ldadd = -pthread \
	$(THIRD_PARTY)/glog/lib@64@/libglog.so

endif

project.targets += liblocalfile_32.a
liblocalfile_32.a.type := library
liblocalfile_32.a.target_bits := 32
liblocalfile_32.a.sources := local_file.cc aioframe/aioframe.cc aioframe/work_thread.cc

project.targets += liblocalfile_32d.a
liblocalfile_32d.a.type := library
liblocalfile_32d.a.debug := y
liblocalfile_32d.a.target_bits := 32
liblocalfile_32d.a.sources := $(liblocalfile_32.a.sources)

ifeq ($(HOSTTYPE),x86_64)
project.targets += liblocalfile_64.a
liblocalfile_64.a.type := library
liblocalfile_64.a.target_bits := 64
liblocalfile_64.a.sources := $(liblocalfile_32.a.sources)

project.targets += liblocalfile_64d.a
liblocalfile_64d.a.type := library
liblocalfile_64d.a.debug := y
liblocalfile_64d.a.target_bits := 64
liblocalfile_64d.a.sources := $(liblocalfile_32.a.sources)
endif

project.targets += liblocalfile_32.so
liblocalfile_32.so.type := shared
liblocalfile_32.so.target_bits := 32
liblocalfile_32.so.sources := $(liblocalfile_32.a.sources)
liblocalfile_32.so.ldadd = -pthread \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase32.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem32.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/text/libwildcard32.so \
	$(THIRD_PARTY)/glog/lib@64@/libglog.so

project.targets += liblocalfile_32d.so
liblocalfile_32d.so.type := shared
liblocalfile_32d.so.debug := y
liblocalfile_32d.so.target_bits := 32
liblocalfile_32d.so.sources := $(liblocalfile_32.a.sources)
liblocalfile_32d.so.ldadd = -pthread \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase32.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem32.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/text/libwildcard32.so \
	$(THIRD_PARTY)/glog/lib@64@/libglog.so

ifeq ($(HOSTTYPE),x86_64)
project.targets += liblocalfile_64.so
liblocalfile_64.so.type := shared
liblocalfile_64.so.target_bits := 64
liblocalfile_64.so.sources := $(liblocalfile_32.a.sources)
liblocalfile_64.so.ldadd = -pthread \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase64.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem64.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/text/libwildcard64.so \
	$(THIRD_PARTY)/glog/lib@64@/libglog.so

project.targets += liblocalfile_64d.so
liblocalfile_64d.so.type := shared
liblocalfile_64d.so.debug := y
liblocalfile_64d.so.target_bits := 64
liblocalfile_64d.so.sources := $(liblocalfile_32.a.sources)
liblocalfile_64d.so.ldadd = -pthread \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase64.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem64.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem64.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/text/libwildcard64.so \
	$(THIRD_PARTY)/glog/lib@64@/libglog.so
endif

ifeq ($(HOSTTYPE),x86_64)
project.targets += file_test_64
project.tests += file_test_64
file_test_64.sources := file_test.cc
file_test_64.target_bits := 64
file_test_64.ldadd = -pthread \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/file/libnewfile_64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/text/libwildcard.a \
	$(THIRD_PARTY)/glog/lib@64@/libglog.a \
	$(THIRD_PARTY)/gflags/lib@64@/libgflags.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

# The final solution will and force link option to the libray.
file_test_64.force_ldadd := $(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/file/liblocalfile_64.a
endif

ifeq ($(HOSTTYPE),x86_64)
project.targets += file_test_64d
project.tests += file_test_64d
file_test_64d.sources := file_test.cc
file_test_64d.target_bits := 64
file_test_64d.debug := y
file_test_64d.ldadd = -pthread \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/file/libnewfile_64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/text/libwildcard.a \
	$(THIRD_PARTY)/glog/lib@64@/libglog.a \
	$(THIRD_PARTY)/gflags/lib@64@/libgflags.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

# The final solution will and force link option to the libray.
file_test_64d.force_ldadd := $(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/file/liblocalfile_64.a
endif

ifeq ($(HOSTTYPE),x86_64)
#project.targets += local_file_test
project.tests += local_file_test
local_file_test.sources := local_file_test.cc
local_file_test.target_bits := 64
local_file_test.ldadd = -pthread \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/file/libnewfile_64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase64.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/text/libwildcard.a \
	$(THIRD_PARTY)/gflags/lib@64@/libgflags.a \
	$(THIRD_PARTY)/glog/lib@64@/libglog.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

# The final solution will and force link option to the libray.
local_file_test.force_ldadd := $(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/file/liblocalfile_64.a
endif

include $(COMMON)/builder/Generic.mak

file_build_finished:
	mkdir -p $(ROOT)/output
	-cp libnewfile_*.a libnewfile_*.so $(ROOT)/output/
	-cp liblocalfile_*.a liblocalfile_*.so  $(ROOT)/output/
