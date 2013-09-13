ROOT=../../../..

project.debug = 1

project.targets = \
	client_test \

project.includes = \
	$(ROOT)/release \
	$(ROOT)/src/thirdparty\
	$(ROOT)/src 

libs=\
	$(ROOT)/release/lib/distribute_lock/lib64/libdist_lock_64.a \
    $(ROOT)/release/lib/zookeeper/lib64/libzookeeper_mt.a \
	$(ROOT)/src/common/base/libbase.a\
	$(ROOT)/src/common/system/libsystem.a \
	$(ROOT)/src/thirdparty/glog/lib64/libglog.a\
	$(ROOT)/src/thirdparty/gflags/lib64/libgflags.a\
	$(ROOT)/src/thirdparty/gtest/lib64/libgtest.a\
	-lpthread

bin_path = ./

client_test.sources   = client_test.cc
client_test.ldadd     = $(libs)
client_test.path     = $(bin_path)


include $(ROOT)/src/common/builder/Generic.mak
