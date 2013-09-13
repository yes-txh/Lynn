# build applipstion
# makefile for ca_server_performance_test

root_path = ../../../../../../
output_path = $(root_path)output
lib_path = $(root_path)output

project.includes = $(root_path)src/ $(root_path)src/thirdparty/
project.debug_symbol = -g
project.libpaths = $(lib_path)
project.defines = 
project.prebuild = ca_server_performance_test_build_prepare
project.postbuild = ca_server_performance_test_build_finished

ca_http_test_client_sources = ca_server_performance_test.cc         \
                              trans_data_thread.cc

builder.verbose = 1


# target 1
project.targets += ca_server_performance_test_64d
ca_server_performance_test_64d.sources = $(ca_http_test_client_sources)
ca_server_performance_test_64d.ldadd = -lpthread  \
                                       $(output_path)/librsa2_64d.a \
                                       $(output_path)/libsvrpublib_64d.a  \
                                       $(root_path)src/thirdparty/glog/lib64/libglog.a \
                                       $(root_path)src/thirdparty/gflags/lib64/libgflags.a \
#                                       $(output_path)/libMonitorClient64.a
ca_server_performance_test_64d.debug = y
ca_server_performance_test_64d.defines = _DEBUG
ca_server_performance_test_64d.target_bits = 64
ca_server_performance_test_64d.path = $(output_path)

# target 2
#project.targets += ca_server_performance_test_64
ca_server_performance_test_64.sources = $(ca_http_test_client_sources)
ca_server_performance_test_64.ldadd = -lpthread \
                                     $(output_path)/librsa2_64.a \
                                     $(output_path)/libsvrpublib_64.a  \
                                     $(root_path)src/thirdparty/glog/lib64/libglog.a \
                                     $(root_path)src/thirdparty/gflags/lib64/libgflags.a \
#                                    $(output_path)/libMonitorClient64.a
ca_server_performance_test_64.target_bits = 64
ca_server_performance_test_64.path = $(output_path)

include $(root_path)src/common/builder/Generic.mak
ca_server_performance_test_build_prepare:
	@echo "________________________________________________________"
	@echo "build ca_server_performance_test* begining."
ca_server_performance_test_build_finished:
	@echo "build ca_server_performance_test* finished."
	@echo "________________________________________________________"
