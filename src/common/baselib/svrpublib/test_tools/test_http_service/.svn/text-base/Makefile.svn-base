# build application
# makefile for test_http_service

root_path = ../../../../../../
output_path = $(root_path)output/

project.includes = $(root_path)src $(root_path)/src/thirdparty/
project.debug_symbol = -g
project.defines = _NEW_FILE_PARSER  _EXT3
#project.libpaths = $(root_path)release/lib/xfs 
project.libpaths = $(output_path)
project.prebuild = test_http_service_build_prepare
project.postbuild = test_http_service_build_finished

test_http_service_sources = my_simple_http.cc    \
                            main.cc

builder.verbose = 1

# target 1
#project.targets += test_http_service 
#test_http_service.sources = $(test_http_service_sources)
#test_http_service.defines = NO_SAVE_CHECK_RESULT
#test_http_service.target_bits = 32
#test_http_service.ldadd = -lsvrpublib_32 -lpthread $(root_path)src/thirdparty/glog/lib/libglog.a $(root_path)src/thirdparty/gflags/lib/libgflags.a -lrt
#test_http_service.path = $(output_path)unit_test/test_http_service
#test_http_service.optimize_flags = -O2

# target 2
#project.targets += test_http_service_d
#test_http_service_d.sources = $(test_http_service_sources)
#test_http_service_d.defines = _DEBUG NO_SAVE_CHECK_RESULT
#test_http_service_d.target_bits = 32
#test_http_service_d.ldadd = -lsvrpublib_32d -lpthread $(root_path)src/thirdparty/glog/lib/libglog.a $(root_path)src/thirdparty/gflags/lib/libgflags.a -lrt
#test_http_service_d.path = $(output_path)unit_test/test_http_service
#test_http_service_d.debug = y

# target 3
#project.targets += test_http_service64
#test_http_service64.sources = $(test_http_service_sources)
#test_http_service64.defines = NO_SAVE_CHECK_RESULT
#test_http_service64.target_bits = 64
#test_http_service64.ldadd = -lsvrpublib_64 -lpthread $(root_path)src/thirdparty/glog/lib64/libglog.a $(root_path)src/thirdparty/gflags/lib64/libgflags.a -lrt
#test_http_service64.path = $(output_path)unit_test/test_http_service
#test_http_service64.optimize_flags = -O2

# target 4
project.targets += test_http_service64_d
test_http_service64_d.sources = $(test_http_service_sources)
test_http_service64_d.defines = _DEBUG NO_SAVE_CHECK_RESULT
test_http_service64_d.target_bits = 64
test_http_service64_d.ldadd = -lrt -lpthread \
                             $(output_path)libsvrpublib_64d.a \
                             $(root_path)src/thirdparty/glog/lib64/libglog.a \
                             $(root_path)src/thirdparty/gflags/lib64/libgflags.a

test_http_service64_d.path = $(output_path)unit_test/test_http_service
test_http_service64_d.debug = y

include $(root_path)src/common/builder/Generic.mak

test_http_service_build_prepare:
	@echo "________________________________________________________"
	@echo "build test_http_service* begining..."

test_http_service_build_finished:
	@echo "build test_http_service* finished."
	@echo "________________________________________________________"
