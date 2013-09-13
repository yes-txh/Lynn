# build application
# makefile for mempool_test

root_path = ../../../../
output_path = $(root_path)output
output_unit_test_path = $(root_path)output

project.includes = $(root_path)src/ 
project.debug_symbol = -g
project.defines = _FILE_OFFSET_BITS=64
project.libpaths =$(output_path) 


# target 1
project.targets += mempool_test
mempool_test.sources = test_hash_table_mempool.cc
mempool_test.defines = GOOGLE_GLOG_DLL_DECL=
mempool_test.target_bits = 32
mempool_test.ldadd = -lsvrpublib -lpthread -lrt $(root_path)src/thirdparty/glog/lib/libglog.a
mempool_test.path = $(output_path)
mempool_test.optimize_flags = -O2

# target 2
project.targets += mempool_test64
mempool_test64.sources = test_hash_table_mempool.cc  
mempool_test64.defines = GOOGLE_GLOG_DLL_DECL=
mempool_test64.target_bits = 64
mempool_test64.ldadd = -lsvrpublib64 -lpthread -lrt  $(root_path)src/thirdparty/glog/lib64/libglog.a
mempool_test64.path = $(output_path)
mempool_test64.optimize_flags = -O2

# target 2 system new
project.targets += mem_new_test
mem_new_test.sources =  test_hash_table_new.cc
mem_new_test.defines = GOOGLE_GLOG_DLL_DECL=
mem_new_test.target_bits = 32
mem_new_test.ldadd = -lsvrpublib -lpthread -lrt  $(root_path)src/thirdparty/glog/lib/libglog.a
mem_new_test.path = $(output_path)
mem_new_test.optimize_flags = -O2



# target 4
project.targets += mem_new_test64
mem_new_test64.sources =  test_hash_table_new.cc
mem_new_test64.defines = GOOGLE_GLOG_DLL_DECL=
mem_new_test64.target_bits = 64
mem_new_test64.ldadd = -lsvrpublib64 -lpthread -lrt  $(root_path)src/thirdparty/glog/lib64/libglog.a
mem_new_test64.path = $(output_path)
mem_new_test64.optimize_flags = -O2

include $(root_path)/src/common/builder/Generic.mak

mempool_test_build_prepare:
	@echo "________________________________________________________"
	@echo "build mempool_test* begining..."

mempool_test_build_finished:
	@echo "build mempool_test* finished."
	@echo "________________________________________________________"

