# makefile for ca_server unittest
rsa_path=$(root_path)release/lib/xfs/

ca_server_unittest_sources = role_manager.cc \
                             ca.cc           \
                             dir_manage.cc
                                    
ca_server_unittest_lib32 = -lpthread  -lz \
                           $(output_path)/libsvrpublib_32.a \
                           $(rsa_path)librsa.a \
                           $(root_path)src/thirdparty/glog/lib/libglog.a \
                           $(root_path)src/thirdparty/protobuf/lib/libprotobuf.a \
                           $(root_path)src/thirdparty/gtest/lib/libgtest.a \
                           $(root_path)src/thirdparty/gflags/lib/libgflags.a 

ca_server_unittest_lib64 = -lpthread -lz \
                           $(output_path)/libsvrpublib_64d.a \
                           $(rsa_path)librsa64.a \
                           $(root_path)src/thirdparty/glog/lib64/libglog.a \
                           $(root_path)src/thirdparty/protobuf/lib64/libprotobuf.a \
                           $(root_path)src/thirdparty/gtest/lib64/libgtest.a \
                           $(root_path)src/thirdparty/gflags/lib64/libgflags.a 

# target 1 convert_records
# target 1_1
#project.targets += convert_records
convert_records.sources = convert_records.cc $(ca_server_unittest_sources)
convert_records.path    = $(output_unit_test_path)
convert_records.target_bits = 32
convert_records.defines = 
convert_records.ldadd = $(ca_server_unittest_lib32)
convert_records.optimize_flags = -O2

# target 1_2
project.targets += convert_records_64d
convert_records_64d.sources = convert_records.cc $(ca_server_unittest_sources)
convert_records_64d.path    = $(output_unit_test_path)
convert_records_64d.target_bits = 64
convert_records_64d.debug = y
convert_records_64d.defines = _DEBUG
convert_records_64d.ldadd = $(ca_server_unittest_lib64)

# target 1 role_manager
# target 1_1
#project.targets += role_manager_test
role_manager_test.sources = role_manager_test.cc $(ca_server_unittest_sources)
role_manager_test.path    = $(output_unit_test_path)
role_manager_test.target_bits = 32
role_manager_test.defines = 
role_manager_test.ldadd = $(ca_server_unittest_lib32)
role_manager_test.optimize_flags = -O2
role_manager_test.flags = --static_off

# target 1_2
project.targets += role_manager_test_64d
role_manager_test_64d.sources = role_manager_test.cc $(ca_server_unittest_sources)
role_manager_test_64d.path    = $(output_unit_test_path)
role_manager_test_64d.target_bits = 64
role_manager_test_64d.debug = y
role_manager_test_64d.defines = _DEBUG
role_manager_test_64d.ldadd = $(ca_server_unittest_lib64)
role_manager_test_64d.flags = --static_off

# target 3 dir_manage
# target 3_1
#project.targets += dir_manage_test
dir_manage_test.sources = dir_manage_test.cc $(ca_server_unittest_sources)
dir_manage_test.path    = $(output_unit_test_path)
dir_manage_test.target_bits = 32
dir_manage_test.defines = 
dir_manage_test.ldadd = $(ca_server_unittest_lib32)
dir_manage_test.optimize_flags = -O2
dir_manage_test.flags = --static_off

# target 3_2
project.targets += dir_manage_test_64d
dir_manage_test_64d.sources = dir_manage_test.cc $(ca_server_unittest_sources)
dir_manage_test_64d.path    = $(output_unit_test_path)
dir_manage_test_64d.target_bits = 64
dir_manage_test_64d.debug = y
dir_manage_test_64d.defines = _DEBUG
dir_manage_test_64d.ldadd = $(ca_server_unittest_lib64)
dir_manage_test_64d.flags = --static_off

# target 4 ca_test
# target 3_1
#project.targets += ca_test
ca_test.sources = ca_test.cc $(ca_server_unittest_sources)
ca_test.path    = $(output_unit_test_path)
ca_test.target_bits = 32
ca_test.defines = 
ca_test.ldadd = $(ca_server_unittest_lib32)
ca_test.optimize_flags = -O2
ca_test.flags = --static_off

# target 3_2
project.targets += ca_test_64d
ca_test_64d.sources = ca_test.cc $(ca_server_unittest_sources)
ca_test_64d.path    = $(output_unit_test_path)
ca_test_64d.target_bits = 64
ca_test_64d.debug = y
ca_test_64d.defines = _DEBUG
ca_test_64d.ldadd = $(ca_server_unittest_lib64)
ca_test_64d.flags = --static_off
