# build application
# makefile for test_mempool 

root_path = ../../../../../../
output_path = $(root_path)output
output_unit_test_path = $(output_path)/unit_test/svrpublib_test

project.includes = $(root_path)src $(root_path)src/thirdparty/
project.debug_symbol = -g
project.defines = 
project.libpaths =$(output_path)
project.prebuild = test_mempool_build_prepare
project.postbuild = test_mempool_build_finished

test_mempool_sources = test_mempool.cc

builder.verbose = 1

# target 1
#project.targets += test_mempool 
#test_mempool.sources = $(test_mempool_sources)
#test_mempool.target_bits = 32
#test_mempool.ldadd = -lpthread -lrt \
                     $(output_path)/libsvrpublib_32.a \
                     $(root_path)src/thirdparty/glog/lib/libglog.a \
					 $(root_path)src/thirdparty/gflags/lib/libgflags.a

#test_mempool.path = $(output_unit_test_path)
#test_mempool.optimize_flags = -O2

# target 2
#project.targets += test_mempool_d
#test_mempool_d.sources = $(test_mempool_sources)
#test_mempool_d.debug = y
#test_mempool_d.defines = _DEBUG
#test_mempool_d.target_bits = 32
#test_mempool_d.ldadd = -lpthread -lrt \
                       $(output_path)/libsvrpublib_32d.a \
                       $(root_path)src/thirdparty/glog/lib/libglog.a \
					   $(root_path)src/thirdparty/gflags/lib/libgflags.a

#test_mempool_d.path = $(output_unit_test_path)

# target 3
#project.targets += test_mempool64
#test_mempool64.sources = $(test_mempool_sources)
#test_mempool64.target_bits = 64
#test_mempool64.ldadd = -lpthread -lrt \
                       $(output_path)/libsvrpublib_64.a \
                       $(root_path)src/thirdparty/glog/lib64/libglog.a \
					   $(root_path)src/thirdparty/gflags/lib64/libgflags.a

#test_mempool64.path = $(output_unit_test_path)
#test_mempool64.optimize_flags = -O2

# target 4
project.targets += test_mempool64_d
test_mempool64_d.sources = $(test_mempool_sources)
test_mempool64_d.debug = y
test_mempool64_d.defines = _DEBUG
test_mempool64_d.target_bits = 64
test_mempool64_d.ldadd = -lpthread -lrt \
                         $(output_path)/libsvrpublib_64d.a \
                         $(root_path)src/thirdparty/glog/lib64/libglog.a \
						 $(root_path)src/thirdparty/gflags/lib64/libgflags.a

test_mempool64_d.path = $(output_unit_test_path)

include $(root_path)/src/common/builder/Generic.mak

test_mempool_build_prepare:
	@echo "________________________________________________________"
	@echo "build test_mempool* begining..."

test_mempool_build_finished:
	@echo "build test_mempool* finished."
	@echo "________________________________________________________"
