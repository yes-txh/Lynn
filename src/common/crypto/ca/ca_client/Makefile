# for libca_client.a and unit test!
ROOT=../../../../../

project.debug = 1
output_unit_test_path = $(ROOT)/output/unit_test/ca_test/

project.includes = \
	$(ROOT)/src \
	$(ROOT)/src/thirdparty\
	$(ROOT)/release/include

libs_32=\
  $(ROOT)/output/libsvrpublib_32.a \
  $(ROOT)/src/common/system/libsystem32.a \
  $(ROOT)/src/thirdparty/glog/lib/libglog.a \
  $(ROOT)/src/thirdparty/gtest/lib/libgtest.a \
  $(ROOT)/src/thirdparty/gflags/lib/libgflags.a \
  -lpthread

libs_32d=\
  $(ROOT)/output/libsvrpublib_32d.a \
  $(ROOT)/src/common/system/libsystem32.a \
  $(ROOT)/src/thirdparty/glog/lib/libglog.a \
  $(ROOT)/src/thirdparty/gtest/lib/libgtest.a \
  $(ROOT)/src/thirdparty/gflags/lib/libgflags.a \
  -lpthread

libs_64=\
  $(ROOT)/output/libsvrpublib_64.a \
  $(ROOT)/src/common/system/libsystem.a \
  $(ROOT)/src/thirdparty/glog/lib64/libglog.a \
  $(ROOT)/src/thirdparty/gtest/lib64/libgtest.a \
  $(ROOT)/src/thirdparty/gflags/lib64/libgflags.a \
  -lpthread

libs_64d=\
  $(ROOT)/output/libsvrpublib_64d.a \
  $(ROOT)/src/common/system/libsystem.a \
  $(ROOT)/src/thirdparty/glog/lib64/libglog.a \
  $(ROOT)/src/thirdparty/gtest/lib64/libgtest.a \
  $(ROOT)/src/thirdparty/gflags/lib64/libgflags.a \
  $(ROOT)/src/common/base/libbase64.a             \
  $(ROOT)/src/common/crypto/libcrypto.a           \
  $(ROOT)/src/common/encoding/libencoding.a       \
  -lpthread

#libca_client_32.a
project.targets += libca_client_32.a
libca_client_32.a.sources = certification.cc
libca_client_32.a.target_bits = 32
libca_client_32.a.type = library

#libca_client_32d.a
project.targets += libca_client_32d.a
libca_client_32d.a.sources = certification.cc
libca_client_32d.a.target_bits = 32
libca_client_32d.a.type = library
libca_client_32d.a.debug = y

#libca_client_64.a
project.targets += libca_client_64.a
libca_client_64.a.sources = certification.cc
libca_client_64.a.target_bits = 64
libca_client_64.a.type = library

#libca_client_64d.a
project.targets += libca_client_64d.a
libca_client_64d.a.sources = certification.cc
libca_client_64d.a.target_bits = 64
libca_client_64d.a.type = library
libca_client_64d.a.debug = y

# certification_test_32
#project.targets += certification_test_32
certification_test_32.sources     = certification.cc certification_test.cc
certification_test_32.ldadd       = $(libs_32)
certification_test_32.path        = $(output_unit_test_path)
certification_test_32.target_bits = 32

# certification_test_64d
project.targets += certification_test_64d
certification_test_64d.sources   = certification.cc certification_test.cc
certification_test_64d.ldadd     = $(libs_64d)
certification_test_64d.path      = $(output_unit_test_path)
certification_test_64d.target_bits = 64
certification_test_64d.debug = y

include $(ROOT)/src/common/builder/Generic.mak
