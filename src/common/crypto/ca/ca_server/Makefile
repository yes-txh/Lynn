# build applipstion
# makefile for ca_server

root_path = ../../../../../
output_path = $(root_path)output
lib_path = $(root_path)output
output_unit_test_path = $(root_path)output/unit_test/ca_server_test

project.includes = $(root_path)src/ $(root_path)src/thirdparty/
project.debug_symbol = -g
project.libpaths = $(lib_path)
project.defines = 
project.prebuild = ca_server_build_prepare
project.postbuild = ca_server_build_finished

ca_server_sources = dir_manage.cc         \
                    ca.cc                 \
                    role_manager.cc       \
                    main.cc               \
                    my_ca_simple_http.cc  \
                    authenticate_record_manager.cc \
					quota_manager.cc
#                    $(root_path)src/common/system/time/time_utils.cpp


builder.verbose = 1

# target 1
#project.targets += ca_server_32 
#ca_server_32.sources = $(ca_server_sources)
#ca_server_32.ldadd = -lpthread \
                  $(output_path)/librsa2_32.a \
                  $(output_path)/libsvrpublib_32.a \
                  $(root_path)src/thirdparty/glog/lib/libglog.a        \
                  $(root_path)src/thirdparty/gflags/lib/libgflags.a
#ca_server_32.defines = 
#ca_server_32.target_bits = 32
#ca_server_32.path = $(output_path)
#ca_server_32.optimize = O2

# target 2
#project.targets += ca_server_32d
#ca_server_32d.sources = $(ca_server_sources)
#ca_server_32d.ldadd = -lpthread \
                    $(output_path)/librsa2_32d.a \
                    $(output_path)/libsvrpublib_32d.a \
                    $(root_path)src/thirdparty/glog/lib/libglog.a          \
                    $(root_path)src/thirdparty/gflags/lib/libgflags.a
#ca_server_32d.debug = y
#ca_server_32d.defines = _DEBUG
#ca_server_32d.target_bits = 32
#ca_server_32d.path = $(output_path)

# target 3
#project.targets += ca_server_64
#ca_server_64.sources = $(ca_server_sources)
#ca_server_64.ldadd = -lpthread \
                    $(output_path)/librsa2_64.a \
                    $(output_path)/libsvrpublib_64.a \
                    $(root_path)src/thirdparty/glog/lib64/libglog.a      \
                    $(root_path)src/thirdparty/gflags/lib64/libgflags.a
#ca_server_64.defines = 
#ca_server_64.target_bits = 64
#ca_server_64.path = $(output_path)
#ca_server_64.optimize = O2

# target 4
project.targets += ca_server_64d
ca_server_64d.sources = $(ca_server_sources)
ca_server_64d.ldadd = -lpthread \
                      $(output_path)/librsa2_64d.a \
                      $(output_path)/libsvrpublib_64d.a \
                      $(root_path)src/thirdparty/glog/lib64/libglog.a        \
                      $(root_path)src/thirdparty/gflags/lib64/libgflags.a    \
                      $(root_path)src/common/base/libbase64.a \
                      $(root_path)src/common/system/libsystem.a
ca_server_64d.debug = y
ca_server_64d.defines = _DEBUG
ca_server_64d.target_bits = 64
ca_server_64d.path = $(output_path)

#include Makefile_test.mak

include $(root_path)src/common/builder/Generic.mak
ca_server_build_prepare:
	@echo "________________________________________________________"
	@echo "build ca_server* begining."
ca_server_build_finished:
	@echo "build ca_server* finished."
	@echo "________________________________________________________"
