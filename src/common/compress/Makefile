ROOT = ../..

project.debug = 1
project.targets = libcompress.a  block_compression_codec_test

libcompress.a.type = lib
libcompress.a.sources = $(wildcard  block_*.cc ./bmz/*.cc ./lzo/*.c ./quicklz/*.cc ./checksum/*.cc)

block_compression_codec_test.sources = block_compression_codec_test.cc
block_compression_codec_test.ldadd = libcompress.a \
                      $(ROOT)/thirdparty/snappy/lib/libsnappy.a\
                      $(ROOT)/thirdparty/gtest/lib@64@/libgtest.a\
                      -lpthread -lz

include $(ROOT)/common/builder/Generic.mak
