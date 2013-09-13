ROOT = ../../..

project.extra_clean = clean_pb_files
project.prebuild = sstable_header.pb.h sstable_header.pb.cc sstable_block_header.pb.h sstable_block_header.pb.cc
project.generated_files = sstable_header.pb.h sstable_header.pb.cc sstable_block_header.pb.h sstable_block_header.pb.cc

project.debug = 1

project.tests := \
                sstable_writer_block_test\
                sstable_reader_block_test\
                sstable_writer_test\
                sstable_reader_test\
                block_cache_test\
				sstable_reader_iterator_test\
				multi_sstables_reader_test\
				multi_readers_iterator_test\
				sstable_merger_test\
				sstable_def_test

project.targets = libsstable.a\
				$(project.tests)

libs = \
	   $(ROOT)/common/distribute_lock/libdist_lock_32.a \
	   $(ROOT)/thirdparty/zookeeper/lib@64@/libzookeeper_mt.a \
	   $(ROOT)/common/crypto/libcrypto.a\
	   $(ROOT)/common/base/libbase.a \
	   $(ROOT)/common/system/libsystem.a \
	   $(ROOT)/common/compress/libcompress.a \
	   $(ROOT)/common/file/libnewfile_32.a\
	   $(ROOT)/common/text/libwildcard.a\
	   $(ROOT)/thirdparty/snappy/lib/libsnappy.a\
	   $(ROOT)/thirdparty/gtest/lib@64@/libgtest.a\
	   $(ROOT)/thirdparty/protobuf/lib@64@/libprotobuf.a \
	   $(ROOT)/thirdparty/glog/lib@64@/libglog.a \
	   $(ROOT)/thirdparty/gflags/lib@64@/libgflags.a \
	   $(ROOT)/thirdparty/openssl/lib@64@/libcrypto.a \
	   -lpthread \
	   -lz \
	   $(ROOT)/common/file/sstable/libsstable.a \

force_ldadd = 	\
	$(ROOT)/common/file/liblocalfile_32.a\

libsstable.a.type = lib
libsstable.a.sources = $(sort $(filter-out %_test.cc, $(wildcard *.cc)) sstable_header.pb.cc sstable_block_header.pb.cc)

sstable_writer_block_test.sources = sstable_writer_block_test.cc
sstable_writer_block_test.ldadd = $(libs)

sstable_reader_block_test.sources = sstable_reader_block_test.cc
sstable_reader_block_test.ldadd = $(libs)

sstable_writer_test.sources = sstable_writer_test.cc
sstable_writer_test.ldadd = $(libs)
sstable_writer_test.force_ldadd = $(force_ldadd)

sstable_reader_test.sources = sstable_reader_test.cc
sstable_reader_test.ldadd = $(libs)
sstable_reader_test.force_ldadd = $(force_ldadd)

block_cache_test.sources = block_cache_test.cc
block_cache_test.ldadd = $(libs)

sstable_reader_iterator_test.sources = sstable_reader_iterator_test.cc
sstable_reader_iterator_test.ldadd = $(libs)
sstable_reader_iterator_test.force_ldadd = $(force_ldadd)

multi_sstables_reader_test.sources = multi_sstables_reader_test.cc
multi_sstables_reader_test.ldadd = $(libs)
multi_sstables_reader_test.force_ldadd = $(force_ldadd)

multi_readers_iterator_test.sources = multi_readers_iterator_test.cc
multi_readers_iterator_test.ldadd = $(libs)
multi_readers_iterator_test.force_ldadd = $(force_ldadd)

sstable_merger_test.sources = sstable_merger_test.cc
sstable_merger_test.ldadd = $(libs)
sstable_merger_test.force_ldadd = $(force_ldadd)

sstable_def_test.sources = sstable_def_test.cc
sstable_def_test.ldadd = $(libs)

include $(ROOT)/common/builder/Generic.mak

sstable_header.pb.h sstable_header.pb.cc : sstable_header.proto
	$(ROOT)/thirdparty/protobuf/bin/protoc --cpp_out=./ sstable_header.proto

sstable_block_header.pb.h sstable_block_header.pb.cc : sstable_block_header.proto
	$(ROOT)/thirdparty/protobuf/bin/protoc --cpp_out=./ sstable_block_header.proto

clean_pb_files :
	rm -f sstable_header.pb.h sstable_header.pb.cc sstable_block_header.pb.h sstable_block_header.pb.cc
