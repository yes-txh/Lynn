ROOT = ../../../..
COMMON = $(ROOT)/common
THIRDPARTY = $(ROOT)/thirdparty

project.extra_warning := -Wno-shadow
project.generated_files := streaming_service.pb.h streaming_service.pb.cc \
						   file_service.pb.h file_service.pb.cc
project.prebuild := $(project.generated_files)
project.extra_clean := clean_generated_files

project.targets += libstreaming.a
libstreaming.a.type = lib
libstreaming.a.sources := $(filter-out %_test.cc file_service.pb.cc,$(sort $(wildcard *.cc) streaming_service.pb.cc))

library := $(COMMON)/base/libbase.a \
		   $(COMMON)/system/libsystem.a \
		   $(COMMON)/netframe/libnetframe.a \
		   $(COMMON)/rpc/proto_rpc/libproto_rpc.a \
		   $(COMMON)/rpc/proto_rpc/streaming/libstreaming.a \
		   $(THIRDPARTY)/glog/lib@64@/libglog.a \
		   $(THIRDPARTY)/gflags/lib@64@/libgflags.a \
		   $(THIRDPARTY)/protobuf/lib@64@/libprotobuf.a \
		   -lpthread

project.targets += test_server
test_server.sources := server_test.cc file_service.pb.cc
test_server.ldadd := $(library)

project.targets += test_client
test_client.sources := client_test.cc file_service.pb.cc
test_client.ldadd := $(library)

include $(COMMON)/builder/Generic.mak

streaming_service.pb.h streaming_service.pb.cc: streaming_service.proto
	$(THIRDPARTY)/protobuf/bin/protoc --cpp_out=. streaming_service.proto
file_service.pb.h file_service.pb.cc: file_service.proto
	$(THIRDPARTY)/protobuf/bin/protoc --cpp_out=. file_service.proto

clean_generated_files :
	@$(RM) $(project.generated_files)
