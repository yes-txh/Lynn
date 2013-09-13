COMMON := ../../

project.targets := rpcc
project.includes := $(COMMON)/..

project.subdirs := .
project.prebuild := parser.yy.cc parser.tab.cc parser.tab.hh
project.extra_clean := clean_generated_files
project.generated_files := parser.yy.cc parser.tab.cc parser.tab.hh
project.debug :=

rpcc.sources  := $(wildcard *.cpp) parser.yy.cc parser.tab.cc
rpcc.ldadd := ../../base/libbase@64@.a
rpcc.path := bin

include $(COMMON)/builder/Generic.mak

parser.tab.cc parser.tab.hh : parser.y
	bison -o parser.tab.cc -d parser.y

parser.yy.cc : parser.l
	flex -o $@ parser.l

clean_generated_files:
	rm -f parser.yy.cc parser.tab.cc parser.tab.hh
