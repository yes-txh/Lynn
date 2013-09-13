ROOT := ../../../..
COMMON := ../..
THIRDPARTY := ../../../thirdparty

project.targets := \
	protoserver protoclient \
	testclient testserver \
	test_loopback \
	rpcclient rpcserver \
	echoclient echoserver \
	multilisten \
	callbackserver callbackclient \
	nestedclient nestedserver \
	pointerserver pointerclient \

project.includes := \
	-I $(COMMON)/.. \
	-I $(THIRDPARTY)

project.debug := 1

libraries := \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/netframe/libnetframe.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/rpc/librpc.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase.a \
	$(THIRDPARTY)/gtest/lib@64@/libgtest.a \
	$(THIRDPARTY)/gtest/lib@64@/libgtest_main.a \
	$(THIRDPARTY)/glog/lib@64@/libglog.a \
	-pthread

callbackserver.sources := callback_server.cpp
callbackserver.ldadd := $(libraries)

callbackclient.sources := callback_client.cpp
callbackclient.ldadd := $(libraries)

testserver.sources := test_server.cpp
testserver.ldadd := $(libraries)

testclient.sources := test_client.cpp
testclient.ldadd := $(libraries)

test_loopback.sources := test_loopback.cpp
test_loopback.ldadd := $(libraries)

echoclient.sources := echo_client.cpp
echoclient.ldadd := $(libraries)

echoserver.sources := echo_server.cpp
echoserver.ldadd := $(libraries)

rpcclient.sources := rpc_client.cpp
rpcclient.ldadd := $(libraries)

rpcserver.sources := rpc_server.cpp
rpcserver.ldadd := $(libraries)

multilisten.sources := multilisten.cpp
multilisten.ldadd := $(libraries)

nestedclient.sources := nested_client.cpp
nestedclient.ldadd := $(libraries)

nestedserver.sources := nested_server.cpp
nestedserver.ldadd := $(libraries)

pointerserver.sources := pointer_server.cpp
pointerserver.ldadd := $(libraries)

pointerclient.sources := pointer_client.cpp
pointerclient.ldadd := $(libraries)

protoserver.sources := proto_server.cpp addressbook.pb.cc
protoserver.ldadd := $(libraries) $(THIRDPARTY)/protobuf/lib@64@/libprotobuf.a

protoclient.sources := proto_client.cpp addressbook.pb.cc
protoclient.ldadd := $(libraries) $(THIRDPARTY)/protobuf/lib@64@/libprotobuf.a

project.generated_files := addressbook.pb.cc

project.prebuild := \
	test_proxy.h test_stub.h \
	multilisten_proxy.h multilisten_stub.h \
	echo_stub.h echo_proxy.h \
	callback_stub.h callback_proxy.h \
	callback_demo_stub.h callback_demo_proxy.h \
	nested_stub.h nested_proxy.h \
	pointer_stub.h pointer_proxy.h \
	addressbook.pb.cc addressbook.pb.h \
	proto_stub.h proto_proxy.h

project.extra_clean := clean_proxy_stub

include $(COMMON)/builder/Generic.mak

test_proxy.h test_stub.h : test.idl ../compiler/bin/rpcc
	../compiler/bin/rpcc --input=$<

echo_stub.h echo_proxy.h : echo.idl ../compiler/bin/rpcc
	../compiler/bin/rpcc --input=$<

callback_stub.h callback_proxy.h : callback.idl ../compiler/bin/rpcc
	../compiler/bin/rpcc --input=$<

callback_demo_stub.h callback_demo_proxy.h : callback_demo.idl ../compiler/bin/rpcc
	../compiler/bin/rpcc --input=$<

multilisten_proxy.h multilisten_stub.h : multilisten.idl ../compiler/bin/rpcc
	../compiler/bin/rpcc --input=$<

nested_stub.h nested_proxy.h : nested.idl ../compiler/bin/rpcc
	../compiler/bin/rpcc --input=$<

pointer_stub.h pointer_proxy.h : pointer.idl ../compiler/bin/rpcc
	../compiler/bin/rpcc --input=$<

addressbook.pb.cc addressbook.pb.h : addressbook.proto $(THIRDPARTY)/protobuf/bin/protoc
	$(THIRDPARTY)/protobuf/bin/protoc --cpp_out=. addressbook.proto

proto_stub.h proto_proxy.h : proto.idl ../compiler/bin/rpcc
	../compiler/bin/rpcc --input=$<

clean_proxy_stub:
	@$(RM) $(project.prebuild)

