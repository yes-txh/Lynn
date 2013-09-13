COMMON = ../../../common
THIRD_PARTY := ../../../thirdparty

project.includes := $(COMMON)/.. $(THIRD_PARTY)
project.targets += libhttp.a
libhttp.a.type := library
libhttp.a.sources := $(filter-out %_test.cc, $(wildcard *.cc))

LIBRARY := \
	$(COMMON)/net/http/libhttp.a\
	$(COMMON)/base/libbase.a\
	$(COMMON)/system/libsystem.a\
	$(COMMON)/netframe/libnetframe.a\
	$(THIRD_PARTY)/gflags/lib@64@/libgflags.a\
	$(THIRD_PARTY)/glog/lib@64@/libglog.a\
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a\
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a\
	-pthread

project.targets += http_message_test
http_message_test.sources := http_message_test.cc
http_message_test.ldadd := $(LIBRARY)

project.targets += http_server_test
http_server_test.sources := http_server_test.cc
http_server_test.ldadd := $(LIBRARY)

project.targets += http_client_test
http_client_test.sources := http_client_test.cc
http_client_test.ldadd := $(LIBRARY)

include $(COMMON)/builder/Generic.mak
