ifndef SOSOBUILD
COMMON := ..
project.predefined_includes := $(COMMON)/.. $(THIRD_PARTY)
endif

project.subdirs = ini xml

project.targets += libconfig.a
libconfig.a.type       := library
libconfig.a.sources    := $(filter-out %test.cc %test.cpp, $(wildcard ini/*.cc xml/*.cpp))

project.targets += libconfig.so
libconfig.so.type := shared
libconfig.so.sources := $(libconfig.a.sources)
libconfig.so.ldadd := \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/base/libbase@64@.so \
	$(if $(SOSO_BUILD),@libroot@,$(COMMON)/..)/common/system/libsystem@64@.so

project.targets += cflags_test
cflags_test.sources := cflags_test.cpp

ini/ini_file.cc.extra_flags := -Wno-format-nonliteral

include $(COMMON)/builder/Generic.mak
