# makefile for svrpublib unittest

TEST_LIBS32=-lpthread -lrt \
                  $(root_path)src/thirdparty/glog/lib/libglog.a \
                  $(root_path)src/thirdparty/gtest/lib/libgtest.a \
                  $(root_path)src/thirdparty/gflags/lib/libgflags.a \
                  $(root_path)output/libsvrpublib_32.a \
                  $(root_path)/src/common/system/libsystem32.a
				  
TEST_LIBS64=-lpthread -lrt \
                  $(root_path)src/thirdparty/glog/lib64/libglog.a \
                  $(root_path)src/thirdparty/gtest/lib64/libgtest.a \
                  $(root_path)src/thirdparty/gflags/lib64/libgflags.a \
                  $(root_path)output/libsvrpublib_64d.a \
                  $(root_path)/src/common/system/libsystem.a

project.includes =$(root_path)src $(root_path)src/thirdparty

## target 1 utf8_test_32
# target 1_1
#project.targets += utf8_test_32
utf8_test_32.sources = ./source/utf8_test.cc
utf8_test_32.path    = $(output_unit_test_path)
utf8_test_32.target_bits = 32
utf8_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
utf8_test_32.optimize_flags = -O2
utf8_test_32.ldadd = $(TEST_LIBS32)

# target 1_2
project.targets += utf8_test_64d
utf8_test_64d.sources = ./source/utf8_test.cc
utf8_test_64d.path    = $(output_unit_test_path)
utf8_test_64d.target_bits = 64
utf8_test_64d.debug = y
utf8_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
utf8_test_64d.ldadd = $(TEST_LIBS64)

## target 2 http_buff_test_32
# target 2_1
#project.targets += http_buff_test_32
http_buff_test_32.sources = ./source/http_buff_test.cc
http_buff_test_32.path    = $(output_unit_test_path)
http_buff_test_32.target_bits = 32
http_buff_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
http_buff_test_32.optimize_flags = -O2
http_buff_test_32.ldadd = $(TEST_LIBS32)

# target 2_2
project.targets += http_buff_test_64d
http_buff_test_64d.sources = ./source/http_buff_test.cc
http_buff_test_64d.path    = $(output_unit_test_path)
http_buff_test_64d.target_bits = 64
http_buff_test_64d.debug = y
http_buff_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF
http_buff_test_64d.ldadd = $(TEST_LIBS64)

## target 3 expandable_array_test_32
# target 3_1
#project.targets += expandable_array_test_32
expandable_array_test_32.sources = ./source/expandable_array_test.cc
expandable_array_test_32.path    = $(output_unit_test_path)
expandable_array_test_32.target_bits = 32
expandable_array_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
expandable_array_test_32.optimize_flags = -O2
expandable_array_test_32.ldadd = $(TEST_LIBS32)

# target 3_2
project.targets += expandable_array_test_64d
expandable_array_test_64d.sources = ./source/expandable_array_test.cc
expandable_array_test_64d.path    = $(output_unit_test_path)
expandable_array_test_64d.target_bits = 64
expandable_array_test_64d.debug = y
expandable_array_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
expandable_array_test_64d.ldadd = $(TEST_LIBS64)

## target 4 key_value_parser_test_32
# target 4_1
#project.targets += key_value_parser_test_32
key_value_parser_test_32.sources = ./source/key_value_parser_test.cc
key_value_parser_test_32.path    = $(output_unit_test_path)
key_value_parser_test_32.target_bits = 32
key_value_parser_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
key_value_parser_test_32.optimize_flags = -O2
key_value_parser_test_32.ldadd = $(TEST_LIBS32)

# target 4_2
project.targets += key_value_parser_test_64d
key_value_parser_test_64d.sources = ./source/key_value_parser_test.cc
key_value_parser_test_64d.path    = $(output_unit_test_path)
key_value_parser_test_64d.target_bits = 64
key_value_parser_test_64d.debug = y
key_value_parser_test_64d.defines = _DEBUG #_MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
key_value_parser_test_64d.ldadd = $(TEST_LIBS64)

## target 5 parser_cgi_parameter_test_32
# target 5_1
#project.targets += parser_cgi_parameter_test_32
parser_cgi_parameter_test_32.sources = ./source/parser_cgi_parameter_test.cc
parser_cgi_parameter_test_32.path    = $(output_unit_test_path)
parser_cgi_parameter_test_32.target_bits = 32
parser_cgi_parameter_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
parser_cgi_parameter_test_32.optimize_flags = -O2
parser_cgi_parameter_test_32.ldadd = $(TEST_LIBS32)

# target 5_2
project.targets += parser_cgi_parameter_test_64d
parser_cgi_parameter_test_64d.sources = ./source/parser_cgi_parameter_test.cc
parser_cgi_parameter_test_64d.path    = $(output_unit_test_path)
parser_cgi_parameter_test_64d.target_bits = 64
parser_cgi_parameter_test_64d.debug = y
parser_cgi_parameter_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
parser_cgi_parameter_test_64d.ldadd = $(TEST_LIBS64)

## target 6 general_util_test_32
# target 6_1
#project.targets += general_util_test_32
general_util_test_32.sources = ./source/general_util_test.cc
general_util_test_32.path    = $(output_unit_test_path)
general_util_test_32.target_bits = 32
general_util_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
general_util_test_32.optimize_flags = -O2
general_util_test_32.ldadd = $(TEST_LIBS32)

# target 6_2
project.targets += general_util_test_64d
general_util_test_64d.sources = ./source/general_util_test.cc
general_util_test_64d.path    = $(output_unit_test_path)
general_util_test_64d.target_bits = 64
general_util_test_64d.debug = y
general_util_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
general_util_test_64d.ldadd = $(TEST_LIBS64)

## target 7 parse_proc_test_32
# target 7_1
#project.targets += parse_proc_test_32
parse_proc_test_32.sources = ./source/parse_proc_test.cc
parse_proc_test_32.path    = $(output_unit_test_path)
parse_proc_test_32.target_bits = 32
parse_proc_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
parse_proc_test_32.optimize_flags = -O2
parse_proc_test_32.ldadd = $(TEST_LIBS32)

# target 7_2
project.targets += parse_proc_test_64d
parse_proc_test_64d.sources = ./source/parse_proc_test.cc
parse_proc_test_64d.path    = $(output_unit_test_path)
parse_proc_test_64d.target_bits = 64
parse_proc_test_64d.debug = y
parse_proc_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
parse_proc_test_64d.ldadd = $(TEST_LIBS64)

## target 8 general_sock_test_32
# target 8_1
#project.targets += general_sock_test_32
general_sock_test_32.sources = ./source/general_sock_test.cc
general_sock_test_32.path    = $(output_unit_test_path)
general_sock_test_32.target_bits = 32
general_sock_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
general_sock_test_32.optimize_flags = -O2
general_sock_test_32.ldadd = $(TEST_LIBS32)

# target 8_2
project.targets += general_sock_test_64d
general_sock_test_64d.sources = ./source/general_sock_test.cc
general_sock_test_64d.path    = $(output_unit_test_path)
general_sock_test_64d.target_bits = 64
general_sock_test_64d.debug = y
general_sock_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
general_sock_test_64d.ldadd = $(TEST_LIBS64)

## target 9 base_protocol_test_32
# target 9_1
#project.targets += base_protocol_test_32
base_protocol_test_32.sources = ./base_protocol_test.cc
base_protocol_test_32.path    = $(output_unit_test_path)
base_protocol_test_32.target_bits = 32
base_protocol_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
base_protocol_test_32.optimize_flags = -O2
base_protocol_test_32.ldadd = $(TEST_LIBS32)

# target 9_2
project.targets += base_protocol_test_64d
base_protocol_test_64d.sources = ./base_protocol_test.cc
base_protocol_test_64d.path    = $(output_unit_test_path)
base_protocol_test_64d.target_bits = 64
base_protocol_test_64d.debug = y
base_protocol_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
base_protocol_test_64d.ldadd = $(TEST_LIBS64)

## target 10 sink_nodes_test_32
# target 10_1
#project.targets += sink_nodes_test_32
sink_nodes_test_32.sources = ./sink_nodes_test.cc
sink_nodes_test_32.path    = $(output_unit_test_path)
sink_nodes_test_32.target_bits = 32
sink_nodes_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
sink_nodes_test_32.optimize_flags = -O2
sink_nodes_test_32.ldadd = $(TEST_LIBS32)

# target 10_2
project.targets += sink_nodes_test_64d
sink_nodes_test_64d.sources = ./sink_nodes_test.cc
sink_nodes_test_64d.path    = $(output_unit_test_path)
sink_nodes_test_64d.target_bits = 64
sink_nodes_test_64d.debug = y
sink_nodes_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
sink_nodes_test_64d.ldadd = $(TEST_LIBS64)

## target 11 epoll_lite_test_32
# target 11_1
#project.targets += epoll_lite_test_32
epoll_lite_test_32.sources = ./source/epoll_lite_test.cc
epoll_lite_test_32.path    = $(output_unit_test_path)
epoll_lite_test_32.target_bits = 32
epoll_lite_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
epoll_lite_test_32.optimize_flags = -O2
epoll_lite_test_32.ldadd = $(TEST_LIBS32)

# target 11_2
project.targets += epoll_lite_test_64d
epoll_lite_test_64d.sources = ./source/epoll_lite_test.cc
epoll_lite_test_64d.path    = $(output_unit_test_path)
epoll_lite_test_64d.target_bits = 64
epoll_lite_test_64d.debug = y
epoll_lite_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
epoll_lite_test_64d.ldadd = $(TEST_LIBS64)

## target 12 general_thread_util_test_32
# target 12_1
#project.targets += general_thread_util_test_32
general_thread_util_test_32.sources = ./source/general_thread_util_test.cc
general_thread_util_test_32.path    = $(output_unit_test_path)
general_thread_util_test_32.target_bits = 32
general_thread_util_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
general_thread_util_test_32.optimize_flags = -O2
general_thread_util_test_32.ldadd = $(TEST_LIBS32)

# target 12_2
project.targets += general_thread_util_test_64d
general_thread_util_test_64d.sources = ./source/general_thread_util_test.cc
general_thread_util_test_64d.path    = $(output_unit_test_path)
general_thread_util_test_64d.target_bits = 64
general_thread_util_test_64d.debug = y
general_thread_util_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
general_thread_util_test_64d.ldadd = $(TEST_LIBS64)

## target 13 simple_http_test_32
# target 13_1
#project.targets += simple_http_test_32
simple_http_test_32.sources = ./source/simple_http_test.cc
simple_http_test_32.path    = $(output_unit_test_path)
simple_http_test_32.target_bits = 32
simple_http_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
simple_http_test_32.optimize_flags = -O2
simple_http_test_32.ldadd = $(TEST_LIBS32)

# target 13_2
project.targets += simple_http_test_64d
simple_http_test_64d.sources = ./source/simple_http_test.cc
simple_http_test_64d.path    = $(output_unit_test_path)
simple_http_test_64d.target_bits = 64
simple_http_test_64d.debug = y
simple_http_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
simple_http_test_64d.ldadd = $(TEST_LIBS64)

## target 14 fake_epoll_test_32
# target 14_1
#project.targets += fake_epoll_test_32
fake_epoll_test_32.sources = ./source/fake_epoll_test.cc
fake_epoll_test_32.path    = $(output_unit_test_path)
fake_epoll_test_32.target_bits = 32
fake_epoll_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
fake_epoll_test_32.optimize_flags = -O2
fake_epoll_test_32.ldadd = $(TEST_LIBS32)

# target 14_2
project.targets += fake_epoll_test_64d
fake_epoll_test_64d.sources = ./source/fake_epoll_test.cc
fake_epoll_test_64d.path    = $(output_unit_test_path)
fake_epoll_test_64d.target_bits = 64
fake_epoll_test_64d.debug = y
fake_epoll_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
fake_epoll_test_64d.ldadd = $(TEST_LIBS64)

## target 15 timed_node_list_test_32
# target 15_1
#project.targets += timed_node_list_test_32
timed_node_list_test_32.sources = ./timed_node_list_test.cc
timed_node_list_test_32.path    = $(output_unit_test_path)
timed_node_list_test_32.target_bits = 32
timed_node_list_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
timed_node_list_test_32.optimize_flags = -O2
timed_node_list_test_32.ldadd = $(TEST_LIBS32)

# target 15_2
project.targets += timed_node_list_test_64d
timed_node_list_test_64d.sources = ./timed_node_list_test.cc
timed_node_list_test_64d.path    = $(output_unit_test_path)
timed_node_list_test_64d.target_bits = 64
timed_node_list_test_64d.debug = y
timed_node_list_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
timed_node_list_test_64d.ldadd = $(TEST_LIBS64)

## target 16 epoll_write_test_32
# target 16_1
#project.targets += epoll_write_test_32
epoll_write_test_32.sources = ./epoll_write_test.cc
epoll_write_test_32.path    = $(output_unit_test_path)
epoll_write_test_32.target_bits = 32
epoll_write_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
epoll_write_test_32.optimize_flags = -O2
epoll_write_test_32.ldadd = $(TEST_LIBS32)

# target 16_2
project.targets += epoll_write_test_64d
epoll_write_test_64d.sources = ./epoll_write_test.cc
epoll_write_test_64d.path    = $(output_unit_test_path)
epoll_write_test_64d.target_bits = 64
epoll_write_test_64d.debug = y
epoll_write_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
epoll_write_test_64d.ldadd = $(TEST_LIBS64)


## target 17 epoll_accept_read_test_32
# target 17_1
#project.targets += epoll_accept_read_test_32
epoll_accept_read_test_32.sources = ./epoll_accept_read_test.cc
epoll_accept_read_test_32.path    = $(output_unit_test_path)
epoll_accept_read_test_32.target_bits = 32
epoll_accept_read_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
epoll_accept_read_test_32.optimize_flags = -O2
epoll_accept_read_test_32.ldadd = $(TEST_LIBS32)

# target 17_2
project.targets += epoll_accept_read_test_64d
epoll_accept_read_test_64d.sources = ./epoll_accept_read_test.cc
epoll_accept_read_test_64d.path    = $(output_unit_test_path)
epoll_accept_read_test_64d.target_bits = 64
epoll_accept_read_test_64d.debug = y
epoll_accept_read_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
epoll_accept_read_test_64d.ldadd = $(TEST_LIBS64)

## target 18 log_test_32
# target 18_1
#project.targets += log_test_32
log_test_32.sources = ./log_test.cc
log_test_32.path    = $(output_unit_test_path)
log_test_32.target_bits = 32
log_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
log_test_32.optimize_flags = -O2
log_test_32.ldadd = $(TEST_LIBS32)

# target 18_2
project.targets += log_test_64d
log_test_64d.sources = ./log_test.cc
log_test_64d.path    = $(output_unit_test_path)
log_test_64d.target_bits = 64
log_test_64d.debug = y
log_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
log_test_64d.ldadd = $(TEST_LIBS64)

## target 19 lite_mempool_test_32
# target 19_1
#project.targets += lite_mempool_test_32
lite_mempool_test_32.sources = ./source/lite_mempool_test.cc
lite_mempool_test_32.path    = $(output_unit_test_path)
lite_mempool_test_32.target_bits = 32
lite_mempool_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
lite_mempool_test_32.optimize_flags = -O2
lite_mempool_test_32.ldadd = $(TEST_LIBS32)

# target 19_2
project.targets += lite_mempool_test_64d
lite_mempool_test_64d.sources = ./source/lite_mempool_test.cc
lite_mempool_test_64d.path    = $(output_unit_test_path)
lite_mempool_test_64d.target_bits = 64
lite_mempool_test_64d.debug = y
lite_mempool_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
lite_mempool_test_64d.ldadd = $(TEST_LIBS64)

## target 20 md5c_test_32
# target 20_1
#project.targets += md5c_test_32
md5c_test_32.sources = ./source/md5c_test.cc
md5c_test_32.path    = $(output_unit_test_path)
md5c_test_32.target_bits = 32
md5c_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
md5c_test_32.optimize_flags = -O2
md5c_test_32.ldadd = $(TEST_LIBS32)

# target 20_2
project.targets += md5c_test_64d
md5c_test_64d.sources = ./source/md5c_test.cc
md5c_test_64d.path    = $(output_unit_test_path)
md5c_test_64d.target_bits = 64
md5c_test_64d.debug = y
md5c_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
md5c_test_64d.ldadd = $(TEST_LIBS64)

## target 21 sha1_test_32
# target 21_1
#project.targets += sha1_test_32
sha1_test_32.sources = ./source/sha1_test.cc
sha1_test_32.path    = $(output_unit_test_path)
sha1_test_32.target_bits = 32
sha1_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
sha1_test_32.optimize_flags = -O2
sha1_test_32.ldadd = $(TEST_LIBS32)

# target 21_2
project.targets += sha1_test_64d
sha1_test_64d.sources = ./source/sha1_test.cc
sha1_test_64d.path    = $(output_unit_test_path)
sha1_test_64d.target_bits = 64
sha1_test_64d.debug = y
sha1_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
sha1_test_64d.ldadd = $(TEST_LIBS64)

## target 22 public_objs_test_32
# target 22_1
#project.targets += public_objs_test_32
public_objs_test_32.sources = ./source/public_objs_test.cc
public_objs_test_32.path    = $(output_unit_test_path)
public_objs_test_32.target_bits = 32
public_objs_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
public_objs_test_32.optimize_flags = -O2
public_objs_test_32.ldadd = $(TEST_LIBS32)

# target 22_2
project.targets += public_objs_test_64d
public_objs_test_64d.sources = ./source/public_objs_test.cc
public_objs_test_64d.path    = $(output_unit_test_path)
public_objs_test_64d.target_bits = 64
public_objs_test_64d.debug = y
public_objs_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
public_objs_test_64d.ldadd = $(TEST_LIBS64)

## target 23 long_conn_test_32
# target 23_1
#project.targets += long_conn_test_32
long_conn_test_32.sources = ./source/long_conn_test.cc
long_conn_test_32.path    = $(output_unit_test_path)
long_conn_test_32.target_bits = 32
long_conn_test_32.defines = _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
long_conn_test_32.optimize_flags = -O2
long_conn_test_32.ldadd = $(TEST_LIBS32)

# target 23_2
project.targets += long_conn_test_64d
long_conn_test_64d.sources = ./source/long_conn_test.cc
long_conn_test_64d.path    = $(output_unit_test_path)
long_conn_test_64d.target_bits = 64
long_conn_test_64d.debug = y
long_conn_test_64d.defines = _DEBUG _MYSQL_SUPPORT_OFF  _THREAD_LOG_OFF _DEBUG_COUNT    AVG_ACCEPT_TCP_OFF 
long_conn_test_64d.ldadd = $(TEST_LIBS64)
