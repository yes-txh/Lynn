# build application
# makefile for longconn_test 

root_path = ../../../../../../../
output_path = $(root_path)output
output_unit_test_path = $(output_path)/unit_test/longconn_test

project.includes = $(root_path)src
project.debug_symbol = -g
project.defines = _LINUX_OS_
project.libpaths =$(output_path)
project.prebuild = longconn_test_build_prepare
project.postbuild = longconn_test_build_finished

longconn_test_client_sources = main.cc            \
                               test_client_obj.cc \
                               test_svr_obj.cc    \
                               longconn_test_thread.cc

builder.verbose = 1

# target 1
project.targets += longconn_test 
longconn_test.sources = $(longconn_test_client_sources)
longconn_test.target_bits = 32
longconn_test.ldadd = -lpthread \
                      $(output_path)/libsvrpublib_32.a \
                      $(root_path)src/thirdparty/glog/lib/libglog.a \
					  $(root_path)src/thirdparty/gflags/lib/libgflags.a
longconn_test.path = $(output_unit_test_path) 
longconn_test.optimize_flags = -O2

# target 2
project.targets += longconn_test_d
longconn_test_d.sources = $(longconn_test_client_sources)
longconn_test_d.debug = y
longconn_test_d.defines = _DEBUG
longconn_test_d.target_bits = 32
longconn_test_d.ldadd = -lpthread \
                        $(output_path)/libsvrpublib_32d.a \
                        $(root_path)src/thirdparty/glog/lib/libglog.a \
						$(root_path)src/thirdparty/gflags/lib/libgflags.a

longconn_test_d.path = $(output_unit_test_path)

# target 3
project.targets += longconn_test64
longconn_test64.sources = $(longconn_test_client_sources)
longconn_test64.target_bits = 64
longconn_test64.ldadd = -lpthread \
                        $(output_path)/libsvrpublib_64.a \
                        $(root_path)src/thirdparty/glog/lib64/libglog.a \
						$(root_path)src/thirdparty/gflags/lib64/libgflags.a

longconn_test64.path = $(output_unit_test_path)
longconn_test64.optimize_flags = -O2

# target 4
project.targets += longconn_test64_d
longconn_test64_d.sources = $(longconn_test_client_sources)
longconn_test64_d.debug = y
longconn_test64_d.defines = _DEBUG
longconn_test64_d.target_bits = 64
longconn_test64_d.ldadd = -lpthread \
                          $(output_path)/libsvrpublib_64d.a \
                          $(root_path)src/thirdparty/glog/lib64/libglog.a \
						  $(root_path)src/thirdparty/gflags/lib64/libgflags.a

longconn_test64_d.path = $(output_unit_test_path)

include $(root_path)/src/common/builder/Generic.mak

longconn_test_build_prepare:
	@echo "________________________________________________________"
	@echo "build longconn_test* begining..."

longconn_test_build_finished:
	@echo "build longconn_test* finished."
	@echo "________________________________________________________"
