ROOT = ../../../../
COMMON = ../..

project.targets := test_shortconn_server test_shortconn_client echo_server echo_client test_echo
project.includes := $(COMMON)/..
project.ldadd := \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/netframe/libnetframe.a \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem.a \
	-pthread

test_shortconn_server.sources = test_shortconn_server.cpp

test_shortconn_client.sources = test_shortconn_client.cpp

echo_server.sources = echo_server.cpp

echo_client.sources = echo_client.cpp

test_echo.sources = test_echo.cpp

include $(COMMON)/builder/Generic.mak
