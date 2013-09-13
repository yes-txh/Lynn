ROOT = ../../../..
COMMON = $(ROOT)/common
THIRDPARTY = $(ROOT)/thirdparty

project.targets := echoserver echoclient
project.prebuild := echo_service.pb.cc echo_service.pb.h
project.generated_files := echo_service.pb.cc
project.prebuild := $(project.generated_files)
project.extra_clean := clean_generated_files
project.extra_warning := -Wno-shadow

libraries := \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/netframe/libnetframe.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/rpc/proto_rpc/libproto_rpc.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase.a \
	$(if $(SOSO_BUILD),@libroot@,$(THIRDPARTY)/..)/thirdparty/gflags/lib@64@/libgflags.a\
	$(if $(SOSO_BUILD),@libroot@,$(THIRDPARTY)/..)/thirdparty/glog/lib@64@/libglog.a\
	-pthread

echoserver.sources := echo_server.cc echo_service.pb.cc
echoserver.ldadd := $(libraries) $(THIRDPARTY)/protobuf/lib@64@/libprotobuf.a

echoclient.sources := echo_client.cc echo_service.pb.cc
echoclient.ldadd := $(libraries) $(THIRDPARTY)/protobuf/lib@64@/libprotobuf.a

include $(COMMON)/builder/Generic.mak

echo_service.pb.cc echo_service.pb.h: echo_service.proto
	$(THIRDPARTY)/protobuf/bin/protoc --cpp_out=$(ROOT) \
		--proto_path=$(ROOT) --proto_path=$(THIRDPARTY) \
		$(ROOT)/common/rpc/proto_rpc/examples/echo_service.proto

clean_generated_files:
	@$(RM) $(project.prebuild)
