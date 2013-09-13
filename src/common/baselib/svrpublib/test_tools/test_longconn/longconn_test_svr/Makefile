# build application
# makefile for longconn_test_svr 

root_path = ../../../../../../../
output_path = $(root_path)output/
output_unit_test_path = $(output_path)unit_test/longconn_test
 
project.includes = $(root_path)src
project.debug_symbol = -g
project.defines = 
project.libpaths = $(output_path)
project.prebuild = longconn_test_svr_build_prepare
project.postbuild = longconn_test_svr_build_finished

longconn_test_svr_sources = main.cc    \
                            test_obj.cc

builder.verbose = 1

# target 4
project.targets += longconn_TEST_svr64_d
longconn_TEST_svr64_d.sources = $(longconn_test_svr_sources)
longconn_TEST_svr64_d.debug = y
longconn_TEST_svr64_d.defines = _DEBUG
longconn_TEST_svr64_d.target_bits = 64
longconn_TEST_svr64_d.path = $(output_unit_test_path)
longconn_TEST_svr64_d.ldadd = -lpthread -lrt \
                              $(output_path)libsvrpublib_64d.a \
                              $(root_path)src/thirdparty/glog/lib64/libglog.a \
                              $(root_path)src/thirdparty/gflags/lib64/libgflags.a

include $(root_path)src/common/builder/Generic.mak

longconn_test_svr_build_prepare:
	@echo "________________________________________________________"
	@echo "build longconn_TEST_svr* begining..."

longconn_test_svr_build_finished:
	@echo "build longconn_TEST_svr* finished."
	@echo "________________________________________________________"
