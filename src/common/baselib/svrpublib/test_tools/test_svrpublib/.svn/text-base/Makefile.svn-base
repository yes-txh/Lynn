# build application
# makefile for test_svrpublib

root_path = ../../../../../../
output_path = $(root_path)output
output_unit_test_path = $(output_path)/unit_test/svrpublib_test

project.includes = $(root_path)src $(root_path)src/thirdparty
project.debug_symbol = -g
project.defines = 
project.libpaths =$(output_path)
project.prebuild = test_svrpublib_prepare
project.postbuild = test_svrpublib_finish

svrpublib_sources = test_thread.cc    \
                    test_svrpublib.cc

builder.verbose = 1

# target 1
#project.targets += test_svrpublib
#test_svrpublib.sources = $(svrpublib_sources)
#test_svrpublib.target_bits = 32
#test_svrpublib.ldadd = -lpthread -lrt \
                       $(output_path)/libsvrpublib_32.a \
                       $(root_path)src/thirdparty/glog/lib/libglog.a \
					   $(root_path)src/thirdparty/gflags/lib/libgflags.a

#test_svrpublib.path = $(output_unit_test_path)
#test_svrpublib.optimize_flags = -O2
#test_svrpublib.flags = --static

# target 2
#project.targets += test_svrpublib_d
#test_svrpublib_d.sources = $(svrpublib_sources)
#test_svrpublib_d.debug = y
#test_svrpublib_d.defines = _DEBUG
#test_svrpublib_d.target_bits = 32
#test_svrpublib_d.ldadd = -lpthread -lrt \
                         $(output_path)/libsvrpublib_32d.a \
                         $(root_path)src/thirdparty/glog/lib/libglog.a \
						 $(root_path)src/thirdparty/gflags/lib/libgflags.a

#test_svrpublib_d.path = $(output_unit_test_path)

# target 3
#project.targets += test_svrpublib64
#test_svrpublib64.sources = $(svrpublib_sources)
#test_svrpublib64.target_bits = 64
#test_svrpublib64.ldadd = -lpthread -lrt \
                         $(output_path)/libsvrpublib_64.a \
                         $(root_path)src/thirdparty/glog/lib64/libglog.a \
						 $(root_path)src/thirdparty/gflags/lib64/libgflags.a

#test_svrpublib64.path = $(output_unit_test_path)
#test_svrpublib64.optimize_flags = -O2

# target 4
project.targets += test_svrpublib64_d
test_svrpublib64_d.sources = $(svrpublib_sources)
test_svrpublib64_d.debug = y
test_svrpublib64_d.defines = _DEBUG
test_svrpublib64_d.target_bits = 64
test_svrpublib64_d.ldadd = -lpthread -lrt \
                           $(output_path)/libsvrpublib_64d.a \
                           $(root_path)src/thirdparty/glog/lib64/libglog.a \
						   $(root_path)src/thirdparty/gflags/lib64/libgflags.a

test_svrpublib64_d.path = $(output_unit_test_path)

include $(root_path)/src/common/builder/Generic.mak

test_svrpublib_prepare:
	@echo "________________________________________________________"
	@echo "build test_svrpublib* begining..."

test_svrpublib_finish:
	@echo "build test_svrpublib* finished."
	@echo "________________________________________________________"

