COMMON := ../..
THIRD_PARTY := ../../../thirdparty

project.targets += print_trace_test
project.tests += print_trace_test
print_trace_test.sources := print_trace_test.cc
print_trace_test.ldadd += -rdynamic

include $(COMMON)/builder/Generic.mak
