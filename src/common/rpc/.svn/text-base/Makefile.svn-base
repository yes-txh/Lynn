-include ./Config.mak

ROOT = ../../..
COMMON = ..
THIRDPARTY=$(COMMON)/../thirdparty

project.targets = librpc.a
project.subdirs = . compiler examples proto_rpc
project.includes = $(COMMON)/.. netframe_channel $(THIRDPARTY)

librpc.a.type = lib
librpc.a.sources = $(wildcard *.cpp) $(wildcard netframe_channel/*.cpp)
librpc.a.ldadd = $(COMMON)/base/libbase.a

project.targets += librpc.so
librpc.so.type = shared
librpc.so.sources = $(librpc.a.sources)
librpc.so.ldadd = \
	-pthread \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase@64@.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem@64@.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/netframe/libnetframe@64@.so

 
include $(COMMON)/builder/Generic.mak
