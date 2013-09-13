//  server_publib.h
//  wookin@tencent
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_SERVER_PUBLIB_H_
#define COMMON_BASELIB_SVRPUBLIB_SERVER_PUBLIB_H_

//
//  support lseeki64()
//
#define _INTEGRAL_MAX_BITS    64



//
//  _MYSQL_SUPPORT:            support MySQL
//

//
//  Xprintf & log file
//  _XPRINTF or _DEBUG output debug info on screen
//  (_XPRINTF or _DEBUG) output debug info to log file
//  (__GetLogFileName() and LOG_MAX_FILESIZE)
//

#include <stdlib.h>

#ifdef _MYSQL_SUPPORT
#include <mysql.h>
// MySQL error message
#include <errmsg.h>
#endif // _MYSQL_SUPPORT

#ifdef NDEBUG
// release
#else
// debug
#ifndef _DEBUG
#define _DEBUG
#endif //
#endif //

#include "common/baselib/svrpublib/twse_type_def.h"
#include "common/baselib/svrpublib/errorcode_svrpublib.h"

//
// Closure
//
#include "common/base/closure.h"

#include "common/baselib/svrpublib/general_head.h"
#include "common/baselib/svrpublib/general_type_def.h"
#include "common/baselib/svrpublib/thread_mutex.h"
#include "common/baselib/svrpublib/wrapper_rwlock.h"

// version
#include "common/baselib/svrpublib/xfs_auto_build_version.h"

#include "common/baselib/svrpublib/general_util.h"
#include "common/baselib/svrpublib/general_thread_util.h"

#include "common/baselib/svrpublib/log.h"
#include "common/baselib/svrpublib/port_event.h"
#include "common/baselib/svrpublib/lite_mempool.h"
#include "common/baselib/svrpublib/general_sock.h"
#include "common/baselib/svrpublib/data_queue.h"

// DEBUG_CGI:show input keys detail on screen
#include "common/baselib/svrpublib/parser_cgi_parameter.h"

//  UTF8 support
#include "common/baselib/svrpublib/utf8.h"

#ifdef _MYSQL_SUPPORT
#include "common/baselib/svrpublib/lite_mysql_wrapper.h"
#endif // _MYSQL_SUPPORT

#include "common/baselib/svrpublib/key_value_parser.h"
#include "common/baselib/svrpublib/base_q.h"
#include "common/baselib/svrpublib/sink_nodes_t.h"

//
//  fake_epoll_xxx() is enable if defined the MACRO _FAKE_EPOLL
//
#include "common/baselib/svrpublib/fake_epoll.h"
#include "common/baselib/svrpublib/epoll_lite.h"
#include "common/baselib/svrpublib/timed_node_list.h"
#include "common/baselib/svrpublib/epoll_write.h"
#include "common/baselib/svrpublib/epoll_accept_read.h"

#include "common/baselib/svrpublib/sha1.h"
#include "common/baselib/svrpublib/md5.h"

#include "common/baselib/svrpublib/base_protocol.h"
#include "common/baselib/svrpublib/expandable_array.h"
//
// Long connection
//
#include "common/baselib/svrpublib/interface_longconn.h"
#include "common/baselib/svrpublib/long_conn.h"

#include "common/baselib/svrpublib/fast_crc.h"
//
// minimum heap
//
#include "common/baselib/svrpublib/min_heap_obj.h"


//
// http server
//
#include "common/baselib/svrpublib/simple_http_proxy.h"
#include "common/baselib/svrpublib/simple_http.h"

// 包含cflags.hpp
// common目中修改cflags路径以后只要修改这一个文件即可
// #include "common/config/cflags.hpp"
// 使用gflags
#include "thirdparty/gflags/gflags.h"
#include "common/baselib/svrpublib/binary_log.h"
#include "common/baselib/svrpublib/binary_log_unpack.h"
#include "common/baselib/svrpublib/http_response.h"
#endif // COMMON_BASELIB_SVRPUBLIB_SERVER_PUBLIB_H_
