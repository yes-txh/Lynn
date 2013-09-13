ROOT=../..
project.debug=1
project.targets = \
	libdist_lock_32.a \
	libdist_lock_32.so

project.includes = \
    $(ROOT) \
    $(ROOT)/thirdparty/

libs = \
	   $(ROOT)/common/system/libsystem@targe_bits@.a\

libso = \
    $(ROOT)/common/system/libsystem@target_bits@.so\
    -lpthread

libdist_lock_32.a.type = lib
libdist_lock_32.a.sources = $(filter-out %_test.cc zookeeper_wrapper.cc, $(wildcard *.cc))
libdist_lock_32.a.target_bits = 32

libdist_lock_32.so.type = shared
libdist_lock_32.so.sources = $(filter-out %_test.cc zookeeper_wrapper.cc, $(wildcard *.cc))
libdist_lock_32.so.target_bits = 32
libdist_lock_32.so.ldadd =  \
	$(libso)\
	$(ROOT)/thirdparty/glog/lib/libglog.so\
	$(ROOT)/thirdparty/zookeeper/lib/libzookeeper_mt.so

ifeq ($(HOSTTYPE),x86_64)
libdist_lock_64.a.type = lib
libdist_lock_64.a.sources = $(filter-out %_test.cc zookeeper_wrapper.cc, $(wildcard *.cc))
libdist_lock_64.a.target_bits = 64

libdist_lock_64.so.type = shared
libdist_lock_64.so.sources = $(filter-out %_test.cc zookeeper_wrapper.cc, $(wildcard *.cc))
libdist_lock_64.so.target_bits = 64
libdist_lock_64.so.ldadd = \
	$(libso) \
	$(ROOT)/thirdparty/glog/lib64/libglog.so\
	$(ROOT)/thirdparty/zookeeper/lib64/libzookeeper_mt.so
endif

ifeq ($(HOSTTYPE),x86_64)
libzookeeper_wrapper_64d.a.type := library
libzookeeper_wrapper_64d.a.target_bits := 64
libzookeeper_wrapper_64d.a.sources := zookeeper_wrapper.cc
libzookeeper_wrapper_64d.a.path = $(ROOT)/output/
endif


project.targets += \
	client_test \
	eph_test \
	list_test \
	lock_test \
	watcher_test  \
	init_test \
	non_block_lock \
	child_test \
	listen_dir \
	rush_node\
	client_local_test \
	tick_test \
	listen_test \
	get_node_data

test_libs=\
	$(ROOT)/common/distribute_lock/libdist_lock_32.a \
	$(ROOT)/thirdparty/zookeeper/lib@64@/libzookeeper_mt.a \
	$(ROOT)/common/base/libbase.a\
	$(ROOT)/common/system/libsystem.a \
	$(ROOT)/thirdparty/gtest/lib@64@/libgtest_main.a \
	$(ROOT)/thirdparty/glog/lib@64@/libglog.a\
	$(ROOT)/thirdparty/gflags/lib@64@/libgflags.a\
	$(ROOT)/thirdparty/gtest/lib@64@/libgtest.a\
	-lpthread

client_test.sources   = client_test.cc
client_test.ldadd     = $(test_libs)

list_test.sources   = list_dir_test.cc
list_test.ldadd     = $(test_libs)

eph_test.sources   = ephe_test.cc
eph_test.ldadd     = $(test_libs)

lock_test.sources   = lock_test.cc
lock_test.ldadd     = $(test_libs)

watcher_test.sources = watcher_test.cc
watcher_test.ldadd   = $(test_libs)

init_test.sources = init_test.cc
init_test.ldadd   = $(test_libs)

non_block_lock.sources = non_block_lock_test.cc
non_block_lock.ldadd   = $(test_libs)

child_test.sources = child_test.cc
child_test.ldadd   = $(test_libs)

listen_dir.sources = listen_dir_test.cc
listen_dir.ldadd   = $(test_libs)

rush_node.sources = rush_node_test.cc
rush_node.ldadd   = $(test_libs)

client_local_test.sources = client_local_test.cc
client_local_test.ldadd   = $(test_libs)

tick_test.sources = tick_test.cc
tick_test.ldadd   = $(test_libs)

listen_test.sources = listen_test.cc
listen_test.ldadd   = $(test_libs)

get_node_data.sources = get_node_data_test.cc
get_node_data.ldadd   = $(test_libs)

include $(ROOT)/common/builder/Generic.mak

update:
	cd ../scripts && ./update_dist_lock.sh && cd - 1>/dev/null 2>&1
