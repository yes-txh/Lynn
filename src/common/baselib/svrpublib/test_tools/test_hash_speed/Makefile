# build application
# makefile for test_hash_speed

root_path = ../../../../../../
output_path = $(root_path)output
output_unit_test_path = $(output_path)/unit_test/svrpublib_test

project.includes = $(root_path)src 
project.debug_symbol = -g
project.defines   = 
project.libpaths  =$(output_path)
project.prebuild  = test_hashspeed_prepare
project.postbuild = test_hashspeed_finish

test_hashspeed_sources = test_hash_speed.cc

builder.verbose = 1

# target 1
#project.targets += test_hash_speed
#test_hash_speed.sources     = $(test_hashspeed_sources)
#test_hash_speed.target_bits = 32
#test_hash_speed.defines     = 
#test_hash_speed.ldadd       = -lsvrpublib -lpthread -lrt $(root_path)src/thirdparty/glog/lib/libglog.a $(root_path)src/thirdparty/gflags/lib/libgflags.a

#test_hash_speed.path        = $(output_unit_test_path)
#test_hash_speed.optimize    = yes
#test_hash_speed.debug       = 
#test_hash_speed.optimize_flags = -O3 -funsigned-bitfields -mno-sse2


# target 2
#project.targets += test_hash_speed_d
#test_hash_speed_d.sources     = $(test_hashspeed_sources)
#test_hash_speed_d.debug       = y
#test_hash_speed_d.defines     = _DEBUG 
#test_hash_speed_d.target_bits = 32
#test_hash_speed_d.ldadd       = -lsvrpublib_d -lpthread -lrt $(root_path)src/thirdparty/glog/lib/libglog.a $(root_path)src/thirdparty/gflags/lib/libgflags.a

#test_hash_speed_d.path        = $(output_unit_test_path)

# target 3
#project.targets += test_hash_speed64
#test_hash_speed64.sources     = $(test_hashspeed_sources)
#test_hash_speed64.target_bits = 64
#test_hash_speed64.defines     = 
#test_hash_speed64.ldadd       = -lsvrpublib64 -lpthread -lrt $(root_path)src/thirdparty/glog/lib64/libglog.a $(root_path)src/thirdparty/gflags/lib64/libgflags.a

#test_hash_speed64.path        = $(output_unit_test_path)
#test_hash_speed64.optimize    = O2

# target 4
project.targets += test_hash_speed64_d
test_hash_speed64_d.sources     = $(test_hashspeed_sources)
test_hash_speed64_d.debug       = y
test_hash_speed64_d.defines     = _DEBUG 
test_hash_speed64_d.target_bits = 64
test_hash_speed64_d.ldadd       = -lsvrpublib64_d -lpthread -lrt $(root_path)src/thirdparty/glog/lib64/libglog.a $(root_path)src/thirdparty/gflags/lib64/libgflags.a

test_hash_speed64_d.path        = $(output_unit_test_path)

include $(root_path)src/common/builder/Generic.mak

test_hashspeed_prepare:
	@echo "________________________________________________________"
	@echo "build test_hash_speed* begining..."

test_hashspeed_finish:
	@echo "build test_hash_speed* finished."
	@echo "________________________________________________________"

