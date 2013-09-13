COMMON=../..

project.subdirs = . test
project.includes := $(COMMON)/..

project.targets += libregex.a
libregex.a.sources = $(wildcard pcre/*.c *.cpp)
libregex.a.type = library

project.targets += libregex.so
libregex.so.type = dynamic
libregex.so.sources = $(wildcard pcre/*.c *.cpp)

include $(COMMON)/builder/Generic.mak
