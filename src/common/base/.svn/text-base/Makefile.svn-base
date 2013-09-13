root_path = ../../..
output_path = $(root_path)/output/common/base

COMMON := ..
THIRD_PARTY := ../../thirdparty

project.subdirs = . serialize string compatible

project.targets :=

project.targets += libbase.a
libbase.a.type := library
libbase.a.sources := \
	$(filter-out %_test.cpp,$(wildcard \
	*.cpp \
	function/*.cpp \
	string/*.cpp \
	compatible/*.cpp \
	module/module.cpp \
	module/dependency_map.cpp \
))

project.targets += libbase.so
libbase.so.type := shared
libbase.so.sources := $(libbase.a.sources)
libbase.so.ldadd = \
	-pthread \
	$(THIRD_PARTY)/gflags/lib@64@/libgflags.so \
	$(THIRD_PARTY)/glog/lib@64@/libglog.so

project.targets += libbase32.a
libbase32.a.type := library
libbase32.a.target_bits := 32
libbase32.a.sources := $(libbase.a.sources)

project.targets += libbase32.so
libbase32.so.type := shared
libbase32.so.target_bits := 32
libbase32.so.sources := $(libbase.a.sources)
libbase32.so.ldadd = \
	-pthread \
	$(THIRD_PARTY)/gflags/lib@64@/libgflags.so \
	$(THIRD_PARTY)/glog/lib@64@/libglog.so

ifeq ($(HOSTTYPE),x86_64)
project.targets += libbase64.a
libbase64.a.type := library
libbase64.a.target_bits := 64
libbase64.a.sources := $(libbase.a.sources)

project.targets += libbase64.so
libbase64.so.type := shared
libbase64.so.target_bits := 64
libbase64.so.sources := $(libbase.a.sources)
libbase64.so.ldadd = \
	-pthread \
	$(THIRD_PARTY)/gflags/lib@64@/libgflags.so \
	$(THIRD_PARTY)/glog/lib@64@/libglog.so
endif

project.targets += closure_test
project.tests += closure_test
closure_test.sources := closure_test.cpp
closure_test.ldadd = \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

project.targets += any_test
project.tests += any_test
any_test.sources := any_test.cpp
any_test.ldadd = \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

project.targets += any_ptr_test
project.test += any_ptr_test
any_ptr_test.sources := any_ptr_test.cpp
any_ptr_test.ldadd = \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

project.targets += byte_order_test
project.tests += byte_order_test
byte_order_test.sources := byte_order_test.cpp
byte_order_test.ldadd := \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

project.targets += class_register_test
project.tests += class_register_test
class_register_test.sources := class_register_test.cc class_register_test_helper.cc
class_register_test.ldadd = -pthread \
                            $(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
                            $(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

project.targets += object_pool_test
project.tests += object_pool_test
object_pool_test.sources := object_pool_test.cpp $(COMMON)/system/concurrency/system_error.cpp
object_pool_test.ldadd := \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

project.targets += uint32_divisor_test
project.tests += uint32_divisor_test
uint32_divisor_test.sources = uint32_divisor_test.cpp
uint32_divisor_test.ldadd = \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

project.targets += array_size_test
project.tests += array_size_test
array_size_test.sources = array_size_test.cpp
array_size_test.ldadd = \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

project.targets += stdint_test
stdint_test.sources := stdint_test.cpp

# project.targets += bind_test
bind_test.sources := bind_test.cc
bind_test.ldadd := \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread

ifdef 0
project.targets += export_variable_test
project.tests += export_variable_test
export_variable_test.sources := export_variable_test.cc
export_variable_test.ldadd += $(COMMON)/system/libsystem.a -lpthread

project.targets += timed_stats_test
timed_stats_test.sources := timed_stats_test.cc
timed_stats_test.ldadd := \
	$(COMMON)/system/libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	-pthread
endif

include $(COMMON)/builder/Generic.mak

closure.h : closure.pl
	./closure.pl > $@

closure2.h : closure2.pl
	./closure2.pl > $@
