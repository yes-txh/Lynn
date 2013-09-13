# build application
# makefile for test_rsa2

#target 1 : rsa2_test
# target 1_1
project.targets += rsa2_test
rsa2_test.sources = $(rsa2_sources) ./rsa_test/rsa2_test.cpp
rsa2_test.defines = 
rsa2_test.target_bits = 32
rsa2_test.ldadd =  -lpthread -lrt 
rsa2_test.path = $(output_unit_test_path)
rsa2_test.optimize_flags = -O2

# target 1_2
project.targets += rsa2_test_d
rsa2_test_d.sources =$(rsa2_sources) ./rsa_test/rsa2_test.cpp
rsa2_test_d.defines = _DEBUG
rsa2_test_d.target_bits = 32
rsa2_test_d.ldadd = -lpthread -lrt 
rsa2_test_d.path = $(output_unit_test_path)
rsa2_test_d.debug = y

# target 1_3
project.targets += rsa2_test64
rsa2_test64.sources = $(rsa2_sources) ./rsa_test/rsa2_test.cpp
rsa2_test64.defines = 
rsa2_test64.target_bits = 64
rsa2_test64.ldadd = -lpthread -lrt 
rsa2_test64.path = $(output_unit_test_path)
rsa2_test64.optimize_flags = -O2

# target 1_4
project.targets += rsa2_test64_d
rsa2_test64_d.sources = $(rsa2_sources) ./rsa_test/rsa2_test.cpp
rsa2_test64_d.defines = _DEBUG
rsa2_test64_d.target_bits = 64
rsa2_test64_d.ldadd = -lpthread -lrt 
rsa2_test64_d.path = $(output_unit_test_path)
rsa2_test64_d.debug = y
