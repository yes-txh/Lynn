COMMON = ../../../common
THIRD_PARTY := ../../../thirdparty

project.includes := $(COMMON)/.. $(THIRD_PARTY)
project.targets += liburi.a
liburi.a.type := library
liburi.a.sources := cgi_params.cpp uri.cpp

include $(COMMON)/builder/Generic.mak
