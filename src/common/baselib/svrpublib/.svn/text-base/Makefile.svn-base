# build library
# makefile for svrpublib

# root path of trunk
root_path = ../../../../
output_path = $(root_path)output/
release_path=$(root_path)release/lib/xfs/
output_unit_test_path = $(output_path)unit_test/svrpublib_test

project.includes =$(root_path)src
project.libpaths = $(output_path)
project.debug_symbol = -g
project.defines =
project.prebuild = svrpublib_build_prepare
project.postbuild = svrpublib_build_finished

project.generated_files = ./source/auto_build_version.cc


svrpublib_sources =./source/fake_epoll.cc          \
                  ./source/general_util.cc         \
                  ./source/lite_mempool.cc         \
                  ./source/md5c.cc                 \
                  ./source/public_objs.cc          \
                  ./source/utf8.cc                 \
                  ./source/epoll_lite.cc           \
                  ./source/general_sock.cc         \
                  ./source/http_buff.cc            \
                  ./source/lite_mysql_wrapper.cc   \
                  ./source/parse_proc.cc           \
                  ./source/sha1.cc                 \
                  ./source/expandable_array.cc     \
                  ./source/general_thread_util.cc  \
                  ./source/key_value_parser.cc     \
                  ./source/long_conn.cc            \
                  ./source/parser_cgi_parameter.cc \
                  ./source/simple_http.cc          \
                  ./source/simple_http_proxy.cc    \
                  ./source/fast_crc.cc             \
                  ./source/http_response.cc        \
                  ./source/http_content.cc         \
                  ./source/auto_build_version.cc   \
                  ./source/generate_cgi_parameter.cc

builder.verbose = 1
./source/fast_crc.cc.extra_flags := -O2

# target 1
project.targets += libsvrpublib_32.a
libsvrpublib_32.a.sources = $(svrpublib_sources)
libsvrpublib_32.a.defines =_MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
libsvrpublib_32.a.target_bits = 32
libsvrpublib_32.a.type = library
libsvrpublib_32.a.path = $(output_path)
libsvrpublib_32.a.optimize_flags = -O2

# target 2
project.targets += libsvrpublib_32d.a
libsvrpublib_32d.a.sources = $(svrpublib_sources)
libsvrpublib_32d.a.defines = _DEBUG _MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
libsvrpublib_32d.a.target_bits = 32
libsvrpublib_32d.a.type = library
libsvrpublib_32d.a.path = $(output_path)
libsvrpublib_32d.a.debug = y

# target 3
project.targets += libsvrpublib_64.a
libsvrpublib_64.a.sources = $(svrpublib_sources)
libsvrpublib_64.a.defines = _MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
libsvrpublib_64.a.target_bits = 64
libsvrpublib_64.a.type = library
libsvrpublib_64.a.path = $(output_path)
libsvrpublib_64.a.optimize_flags = -O2

# target 4
project.targets += libsvrpublib_64d.a
libsvrpublib_64d.a.sources = $(svrpublib_sources)
libsvrpublib_64d.a.debug = y
libsvrpublib_64d.a.defines = _DEBUG _MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
libsvrpublib_64d.a.target_bits = 64
libsvrpublib_64d.a.type = library
libsvrpublib_64d.a.path = $(output_path)


# target 5
project.targets += libsvrpublib_32.so
libsvrpublib_32.so.sources = $(svrpublib_sources)
libsvrpublib_32.so.defines =_MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
libsvrpublib_32.so.target_bits = 32
libsvrpublib_32.so.type = shared
libsvrpublib_32.so.path = $(output_path)
libsvrpublib_32.so.optimize_flags = -O2
libsvrpublib_32.so.ldadd = -lpthread -lrt \
                           $(root_path)src/thirdparty/glog/lib/libglog.so                  \
                           $(root_path)src/thirdparty/gflags/lib/libgflags.so              \
                           $(root_path)src/common/system/libsystem32.so

# target 6
project.targets += libsvrpublib_32d.so
libsvrpublib_32d.so.sources = $(svrpublib_sources)
libsvrpublib_32d.so.defines = _DEBUG _MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
libsvrpublib_32d.so.target_bits = 32
libsvrpublib_32d.so.type = shared
libsvrpublib_32d.so.path = $(output_path)
libsvrpublib_32d.so.debug = y
libsvrpublib_32d.so.ldadd = -lpthread
libsvrpublib_32d.so.ldadd = -lpthread -lrt \
                            $(root_path)src/thirdparty/glog/lib/libglog.so                 \
                            $(root_path)src/thirdparty/gflags/lib/libgflags.so             \
                            $(root_path)src/common/system/libsystem32.so

# target 7
project.targets += libsvrpublib_64.so
libsvrpublib_64.so.sources = $(svrpublib_sources)
libsvrpublib_64.so.defines = _MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
libsvrpublib_64.so.target_bits = 64
libsvrpublib_64.so.type = shared
libsvrpublib_64.so.path = $(output_path)
libsvrpublib_64.so.optimize_flags = -O2
libsvrpublib_64.so.ldadd = -lpthread
libsvrpublib_64.so.ldadd = -lpthread -lrt \
                           $(root_path)src/thirdparty/glog/lib64/libglog.so                  \
                           $(root_path)src/thirdparty/gflags/lib64/libgflags.so              \
                           $(root_path)src/common/system/libsystem.so
# target 8
project.targets += libsvrpublib_64d.so
libsvrpublib_64d.so.sources = $(svrpublib_sources)
libsvrpublib_64d.so.debug = y
libsvrpublib_64d.so.defines = _DEBUG _MYSQL_SUPPORT_OFF  _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
libsvrpublib_64d.so.target_bits = 64
libsvrpublib_64d.so.type = shared
libsvrpublib_64d.so.path = $(output_path)
libsvrpublib_64d.so.ldadd = -lpthread
libsvrpublib_64d.so.ldadd = -lpthread -lrt \
                            $(root_path)src/thirdparty/glog/lib64/libglog.so                  \
                            $(root_path)src/thirdparty/gflags/lib64/libgflags.so              \
                            $(root_path)src/common/system/libsystem.so
include Makefile_test.mak
include $(root_path)/src/common/builder/Generic.mak

./source/auto_build_version.cc:
	@echo "check in svrpublib_build_prepare"

svrpublib_build_prepare:
	@echo "________________________________________________________"
	@echo "build svrpublib*.a begining..."
	@echo; if [ ! -e ./source/auto_build_version.cc ]; then cp -f ./source/auto_build_version.inl ./source/auto_build_version.cc; fi
	@echo; find_len=$$(more ./source/auto_build_version.cc | grep xfs_auto_build_version.h | wc -c); if [ $${find_len} -eq 0 ]; then	echo 'incorrect auto_build_version.cc';	cp -f ./source/auto_build_version.inl ./source/auto_build_version.cc; fi

svrpublib_build_finished:
	@echo "build svrpublib*.a finished."
	@echo "________________________________________________________"
