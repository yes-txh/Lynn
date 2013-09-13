COMMON := ../../
THIRD_PARTY := ../../../thirdparty

project.debug    := 1
project.includes := $(COMMON).. $(THIRD_PARTY)

project.targets += io_test
project.tests += io_test
io_test.sources = io_test.cpp
io_test.ldadd := \
	-pthread \
	../../text/libwildcard.a \
	../libsystem.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a

include $(COMMON)builder/Generic.mak
