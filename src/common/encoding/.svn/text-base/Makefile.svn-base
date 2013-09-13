ROOT = ../../..
COMMON = ..
THIRD_PARTY := ../../thirdparty
project.includes = $(COMMON)/.. $(THIRD_PARTY)

project.extra_warning = -Wno-float-equal
project.optimize_flags = -O3
project.ldadd := \
	-pthread \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest_main.a \
	$(THIRD_PARTY)/gtest/lib@64@/libgtest.a

project.targets += libencoding.a
libencoding.a.type := library
libencoding.a.sources := \
	$(wildcard modp_b64/*.cpp) \
	base64.cpp \
	percent.cpp

project.targets += variant_integer_test
project.tests += variant_integer_test
variant_integer_test.sources = variant_integer_test.cpp

project.targets += ascii_test
project.tests += ascii_test
ascii_test.sources = ascii_test.cpp

project.targets += gb_test
project.tests += gb_test
gb_test.sources = gb_test.cpp

project.targets += hex_test
project.tests += hex_test
hex_test.sources = hex_test.cpp

project.targets += percent_test
project.tests += percent_test
percent_test.sources = percent_test.cpp
percent_test.ldadd = libencoding.a

project.targets += base64_test
project.tests += base64_test
base64_test.sources := base64_test.cpp base64.cpp $(wildcard modp_b64/*.cpp)


include $(COMMON)/builder/Generic.mak
