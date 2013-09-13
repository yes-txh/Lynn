ROOT = ../../..
COMMON = ..

project.includes = $(COMMON)/..
project.subdirs = . test

project.targets += libnetframe.a
libnetframe.a.type := library
libnetframe.a.sources := \
	command_queue.cpp \
	epoll_event_poller.cpp \
	listen_socket_context.cpp \
	socket_context.cpp \
	work_thread.cpp \
	datagram_socket_context.cpp \
	netframe.cpp \
	stream_socket_context.cpp \
	wsa_event_poller.cpp

project.targets += libnetframe32.a
libnetframe32.a.type := library
libnetframe32.a.target_bits := 32
libnetframe32.a.sources := $(libnetframe.a.sources)

project.targets += libnetframe.so
libnetframe.so.type := shared
libnetframe.so.sources := $(libnetframe.a.sources)
libnetframe.so.ldadd := \
	-pthread \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase@target_bits@.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem@target_bits@.so

project.targets += libnetframe32.so
libnetframe32.so.type := shared
libnetframe32.so.target_bits := 32
libnetframe32.so.sources := $(libnetframe.a.sources)
libnetframe32.so.ldadd := $(libnetframe.so.ldadd)

ifeq ($(HOSTTYPE),x86_64)
project.targets += libnetframe64.a
libnetframe64.a.type := library
libnetframe64.a.target_bits := 64
libnetframe64.a.sources := $(libnetframe.a.sources)

project.targets += libnetframe64.so
libnetframe64.so.type := shared
libnetframe64.so.target_bits := 64
libnetframe64.so.sources := $(libnetframe.a.sources)
libnetframe64.so.ldadd := $(libnetframe.so.ldadd)
libnetframe64.so.ldadd := \
	-pthread \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase@target_bits@.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem@target_bits@.so
endif

include $(COMMON)/builder/Generic.mak
