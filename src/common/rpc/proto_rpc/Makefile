ROOT = ../../..
COMMON = $(ROOT)/common
THIRDPARTY = $(ROOT)/thirdparty

project.targets += libproto_rpc.a
project.extra_warning := -Wno-shadow
project.generated_files := rpc_meta.pb.h rpc_meta.pb.cc \
						   rpc_option.pb.h rpc_option.pb.cc
project.prebuild := $(project.generated_files)
project.extra_clean := clean_generated_files
project.subdirs = . examples streaming

libproto_rpc.a.type = lib
libproto_rpc.a.sources = http_message.cc \
						 http_service.cc \
						 rpc_controller.cc \
						 rpc_request_queue.cc \
						 rpc_service.cc \
						 rpc_session_pool.cc \
						 rpc_meta.pb.cc \
						 rpc_option.pb.cc

project.targets += libproto_rpc32.a
libproto_rpc32.a.type = lib
libproto_rpc32.a.target_bits := 32
libproto_rpc32.a.sources = $(libproto_rpc.a.sources)

project.targets += libproto_rpc.so
libproto_rpc.so.type = shared
libproto_rpc.so.sources = $(libproto_rpc.a.sources)
libproto_rpc.so.ldadd = $(COMMON)/netframe/libnetframe.so\
                        $(COMMON)/system/libsystem.so\
                        $(COMMON)/base/libbase.so\
                        $(THIRDPARTY)/glog/lib@64@/libglog.so\
                        $(THIRDPARTY)/protobuf/lib@64@/libprotobuf.so\
                        -lpthread

ifeq ($(HOSTTYPE),x86_64)
project.targets += libproto_rpc64.so
libproto_rpc64.so.type = shared
libproto_rpc64.so.target_bits := 64
libproto_rpc64.so.sources = $(libproto_rpc.a.sources)
libproto_rpc64.so.ldadd = $(COMMON)/netframe/libnetframe64.so\
                        $(THIRDPARTY)/glog/lib64/libglog.so\
                        $(COMMON)/system/libsystem64.so\
                        $(THIRDPARTY)/protobuf/lib64/libprotobuf.so\
                        $(COMMON)/base/libbase64.so\
                        -lpthread
endif

project.targets += libproto_rpc32.so
libproto_rpc32.so.type = shared
libproto_rpc32.so.target_bits = 32
libproto_rpc32.so.sources = $(libproto_rpc.a.sources)
libproto_rpc32.so.ldadd = $(COMMON)/netframe/libnetframe32.so\
                          $(THIRDPARTY)/glog/lib/libglog.so\
                          $(COMMON)/system/libsystem32.so\
                          $(THIRDPARTY)/protobuf/lib/libprotobuf.so\
                          $(COMMON)/base/libbase32.so\
                          -lpthread

include $(COMMON)/builder/Generic.mak

rpc_meta.pb.h rpc_meta.pb.cc: rpc_meta.proto
	$(THIRDPARTY)/protobuf/bin/protoc --cpp_out=. rpc_meta.proto

# As some other modules depends on rpc_option.proto, it must be compiled in the
# whole project tree, or it can't be initialized correctly.
rpc_option.pb.h rpc_option.pb.cc: rpc_option.proto
	$(THIRDPARTY)/protobuf/bin/protoc --cpp_out=$(ROOT) \
		--proto_path=$(ROOT) --proto_path=$(THIRDPARTY) \
		$(ROOT)/common/rpc/proto_rpc/rpc_option.proto

clean_generated_files :
	@$(RM) $(project.generated_files)
