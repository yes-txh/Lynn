COMMON := ../..
THIRDPARTY := ../../../thirdparty

project.targets += cpu_usage_test
cpu_usage_test.sources = cpu_usage_test.cpp
cpu_usage_test.ldadd = \
	$(COMMON)/base/libbase@64@.a \
	$(COMMON)/system/libsystem.a \
	$(THIRDPARTY)/gflags/lib@64@/libgflags.a

include $(COMMON)/builder/Generic.mak
