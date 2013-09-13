# build application
# makefile for test_rawlog

root_path = ../../../../../../
output_path = $(root_path)output
output_unit_test_path = $(output_path)/unit_test/svrpublib_test

project.includes = $(root_path)src
project.debug_symbol = -g
project.defines   = 
project.libpaths  =$(output_path)
project.prebuild  = test_rawlog_prepare
project.postbuild = test_rawlog_finish

test_rawlog_sources = test_rawlog.cc

builder.verbose = 1

# target 1
#project.targets += test_rawlog
#test_rawlog.sources     = $(test_rawlog_sources)
#test_rawlog.target_bits = 32
#test_rawlog.defines     = 
#test_rawlog.ldadd       = -lpthread -lrt \
                          $(output_path)/libsvrpublib_32.a \
                          $(root_path)src/thirdparty/glog/lib/libglog.a \
						  $(root_path)src/thirdparty/gflags/lib/libgflags.a

#test_rawlog.path        = $(output_unit_test_path)
#test_rawlog.optimize    = O2

# target 2
#project.targets += test_rawlog_d
#test_rawlog_d.sources     = $(test_rawlog_sources)
#test_rawlog_d.debug       = y
#test_rawlog_d.defines     = _DEBUG 
#test_rawlog_d.target_bits = 32
#test_rawlog_d.ldadd       = -lpthread -lrt \
                            $(output_path)/libsvrpublib_32d.a \
                            $(root_path)src/thirdparty/glog/lib/libglog.a \
							$(root_path)src/thirdparty/gflags/lib/libgflags.a

#test_rawlog_d.path        = $(output_unit_test_path)

# target 3
#project.targets += test_rawlog64
#test_rawlog64.sources     = $(test_rawlog_sources)
#test_rawlog64.target_bits = 64
#test_rawlog64.defines     = 
#test_rawlog64.ldadd       = -lpthread -lrt \
                            $(output_path)/libsvrpublib_64.a \
                            $(root_path)src/thirdparty/glog/lib64/libglog.a \
							$(root_path)src/thirdparty/gflags/lib64/libgflags.a

#test_rawlog64.path        = $(output_unit_test_path)
#test_rawlog64.optimize    = O2

# target 4
project.targets += test_rawlog64_d
test_rawlog64_d.sources     = $(test_rawlog_sources)
test_rawlog64_d.debug       = y
test_rawlog64_d.defines     = _DEBUG 
test_rawlog64_d.target_bits = 64
test_rawlog64_d.ldadd       = -lpthread -lrt \
                              $(output_path)/libsvrpublib_64d.a \
                              $(root_path)src/thirdparty/glog/lib64/libglog.a \
							  $(root_path)src/thirdparty/gflags/lib64/libgflags.a

test_rawlog64_d.path        = $(output_unit_test_path)

include $(root_path)/src/common/builder/Generic.mak

test_rawlog_prepare:
	@echo "________________________________________________________"
	@echo "build test_rawlog* begining..."

test_rawlog_finish:
	@echo "build test_rawlog* finished."
	@echo "________________________________________________________"

