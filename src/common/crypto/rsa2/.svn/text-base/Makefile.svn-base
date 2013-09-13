# build library
# makefile for rsa

# root path of trunk
root_path = ../../../../
output_path = $(root_path)output/
output_unit_test_path = $(output_path)/unit_test/rsa2_test

project.includes =$(root_path)src
project.libpaths = $(output_path)
project.debug_symbol = -g
project.defines = 
project.prebuild = rsa2_build_prepare
project.postbuild = rsa2_build_finished


rsa2_sources =./rsa2_ref/rsa_crypt.cpp               \
              ./rsa2_ref/rsa_source/desc.cpp         \
              ./rsa2_ref/rsa_source/digit.cpp        \
              ./rsa2_ref/rsa_source/md2c.cpp         \
              ./rsa2_ref/rsa_source/nn.cpp           \
              ./rsa2_ref/rsa_source/prime.cpp        \
              ./rsa2_ref/rsa_source/r_encode.cpp     \
              ./rsa2_ref/rsa_source/r_enhanc.cpp     \
              ./rsa2_ref/rsa_source/r_keygen.cpp     \
              ./rsa2_ref/rsa_source/r_random.cpp     \
              ./rsa2_ref/rsa_source/r_stdlib.cpp     \
              ./rsa2_ref/rsa_source/rsa.cpp          \
              ./rsa2_ref/rsa_source/rsa_md5c.cpp    

builder.verbose = 1

# target 1
#project.targets += librsa2_32.a
librsa2_32.a.sources = $(rsa2_sources)
librsa2_32.a.defines =_MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
librsa2_32.a.target_bits = 32
librsa2_32.a.type = library
librsa2_32.a.path = $(output_path)
librsa2_32.a.optimize_flags = -O2

# target 2
#project.targets += librsa2_32d.a
librsa2_32d.a.sources = $(rsa2_sources)
librsa2_32d.a.defines = _DEBUG _MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
librsa2_32d.a.target_bits = 32
librsa2_32d.a.type = library
librsa2_32d.a.path = $(output_path)
librsa2_32d.a.debug = y

# target 3
#project.targets += librsa2_64.a
librsa2_64.a.sources = $(rsa2_sources)
librsa2_64.a.defines = _MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
librsa2_64.a.target_bits = 64
librsa2_64.a.type = library
librsa2_64.a.path = $(output_path)
librsa2_64.a.optimize_flags = -O2

# target 4
project.targets += librsa2_64d.a
librsa2_64d.a.sources = $(rsa2_sources)
librsa2_64d.a.debug = y
librsa2_64d.a.defines = _DEBUG _MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
librsa2_64d.a.target_bits = 64
librsa2_64d.a.type = library
librsa2_64d.a.path = $(output_path)

include $(root_path)src/common/builder/Generic.mak

rsa2_build_prepare:
	@echo "________________________________________________________"
	@echo "build rsa*.a begining..."

rsa2_build_finished:
	@echo "build rsa*.a finished."
	@echo "________________________________________________________"	
