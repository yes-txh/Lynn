# build application
# makefile for test_binary_log

root_path = ../../../../../../
output_path = $(root_path)output
output_unit_test_path = $(output_path)/unit_test/svrpublib_test

project.includes = $(root_path)src
project.debug_symbol = -g
project.defines = 
project.libpaths =$(output_path)
project.prebuild = test_binary_log_build_prepare
project.postbuild = test_binary_log_build_finished

test_binary_log_sources = test_binary_log.cc

builder.verbose = 1

# target 1
#project.targets += test_binary_log 
#test_binary_log.sources = $(test_binary_log_sources)
#test_binary_log.target_bits = 32
#test_binary_log.ldadd = -lsvrpublib_32 -lpthread -lrt $(root_path)src/thirdparty/glog/lib/libglog.a \
                        $(root_path)src/thirdparty/gflags/lib/libgflags.a
#test_binary_log.path = $(output_unit_test_path)
#test_binary_log.optimize_flags = -O2

# target 2
#project.targets += test_binary_log_d
#test_binary_log_d.sources = $(test_binary_log_sources)
#test_binary_log_d.debug = y
#test_binary_log_d.defines = _DEBUG
#test_binary_log_d.target_bits = 32
#test_binary_log_d.ldadd = -lsvrpublib_32d -lpthread -lrt $(root_path)src/thirdparty/glog/lib/libglog.a \
                          $(root_path)src/thirdparty/gflags/lib/libgflags.a
#test_binary_log_d.path = $(output_unit_test_path)

# target 3
#project.targets += test_binary_log64
#test_binary_log64.sources = $(test_binary_log_sources)
#test_binary_log64.target_bits = 64
#test_binary_log64.ldadd = -lsvrpublib_64 -lpthread -lrt $(root_path)src/thirdparty/glog/lib64/libglog.a \
                          $(root_path)src/thirdparty/gflags/lib64/libgflags.a
#test_binary_log64.path = $(output_unit_test_path)
#test_binary_log64.optimize_flags = -O2

# target 4
project.targets += test_binary_log64_d
test_binary_log64_d.sources = $(test_binary_log_sources)
test_binary_log64_d.debug = y
test_binary_log64_d.defines = _DEBUG
test_binary_log64_d.target_bits = 64
test_binary_log64_d.ldadd = -lsvrpublib_64d -lpthread -lrt $(root_path)src/thirdparty/glog/lib64/libglog.a \
                            $(root_path)src/thirdparty/gflags/lib64/libgflags.a
test_binary_log64_d.path = $(output_unit_test_path)

include $(root_path)/src/common/builder/Generic.mak

test_binary_log_build_prepare:
	@echo "________________________________________________________"
	@echo "build test_binary_log* begining..."

test_binary_log_build_finished:
	@echo "build test_binary_log* finished."
	@echo "________________________________________________________"
