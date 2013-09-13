// errorcode_svrpublib.h
// wookin@tencent.com
// ----------------------------------------------------

#ifndef COMMON_BASELIB_SVRPUBLIB_ERRORCODE_SVRPUBLIB_H_
#define COMMON_BASELIB_SVRPUBLIB_ERRORCODE_SVRPUBLIB_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

enum ERROR_CODE {
    ERROR_FAIL = 0,
    ERROR_OK = 1,

    //  //////////////////////////////////////
    //  Receive data
    //  //////////////////////////////////////
    ERROR_RECV_STATE_BEGIN = 10,
    ERROR_RECV_STATE_ERROR = ERROR_RECV_STATE_BEGIN,
    ERROR_RECV_STATE_TIMEOUT,
    ERROR_RECV_STATE_CLIENT_CLOSED,
    ERROR_RECV_STATE_END_FLAG,
    ERROR_RECV_STATE_CHECK_PACK_FAIL,
    // Request package is too Long
    ERROR_RECV_STATE_ERR_TOO_LONG_REQ,
    ERROR_RECV_STATE_END = 100,

    //  //////////////////////////////////////
    ERROR_PROCESS_BEGIN = 101,
    ERROR_PROCESS_DOWNSTREAM_BUSY = ERROR_PROCESS_BEGIN,
    ERROR_PROCESS_EPOLL_FD_ERROR = 51,
    ERROR_PROCESS_END = 500,

    //  //////////////////////////////////////
    //  Send data state
    //  //////////////////////////////////////
    ERROR_SEND_STATE_BEGIN = 501,
    ERROR_SEND_STATE_ERROR = ERROR_SEND_STATE_BEGIN,
    ERROR_SEND_STATE_NETWORK_BUSY,
    ERROR_SEND_STATE_TRY_AGAIN,
    ERROR_SEND_STATE_EWOULDBLOCK,
    ERROR_SEND_STATE_FINISHED,
    ERROR_SEND_STATE_END = 1000,

    //  //////////////////////////////////////
    //  CSocketIOThread_T
    //  //////////////////////////////////////
    ERROR_SOCKETIO_GROUPTASK_BEGIN = 1001,

    //  //////////////////////////////////////
    //  Group task error codes
    //  //////////////////////////////////////
    ERROR_SOCKETIO_GROUPTASK_ALLFAIL_TIMEOUT,
    ERROR_SOCKETIO_GROUPTASK_PART_SUCCESS,
    ERROR_SOCKETIO_GROUPTASK_FULL_SUCCESS,

    //  //////////////////////////////////////
    ERROR_SOCKETIO_BEGIN = 1100,
    ERROR_SOCKETIO_DEAL_NODE_FAIL = ERROR_SOCKETIO_BEGIN,
    ERROR_SOCKETIO_ADD_NODE_TO_TIMED_LIST_FAIL,
    ERROR_SOCKETIO_RECV_REQUEST_TIMEOUT,
    ERROR_SOCKETIO_SEND_RESPONSE_TIMEOUT,
    ERROR_SOCKETIO_SEND_REQUEST_TIMEOUT,
    ERROR_SOCKETIO_RECV_RESPONSE_TIMEOUT,
    ERROR_SOCKETIO_INVALID_REQUEST_PACKLEN,
    ERROR_SOCKETIO_FORCE_END_ALL_SESSIONS,
    // epoll
    ERROR_SOCKETIO_ADD_TO_EPOLL_FAIL,
    // ...
    ERROR_SOCKETIO_END = 1500,

    //  //////////////////////////////////////
    //  Business error code
    //  //////////////////////////////////////
    ERROR_BUSINESS_BEGIN = 1501,
    ERROR_BUSINESS_PREPARE_FD_TO_DOWNSTREAM_FAIL,
    // ...
    ERROR_BUSINESS_END = 2000,

    ERROR_SVRPUB_END = 10000,
    //
};

enum GROUP_ERR {
    GERR_ALLFAIL_TIMEOUT = ERROR_SOCKETIO_GROUPTASK_ALLFAIL_TIMEOUT,
    GERR_PART_SUCCESS = ERROR_SOCKETIO_GROUPTASK_PART_SUCCESS,
    GERR_FULL_SUCCESS = ERROR_SOCKETIO_GROUPTASK_FULL_SUCCESS,
};

typedef ERROR_CODE    ENUM_RECV_STATE;
typedef ERROR_CODE    ENUM_SEND_STATE;

inline const char* GetErrorCodeName(ERROR_CODE err) {
    switch (err) {
    case ERROR_FAIL:
        return "ERROR_FAIL";

    case ERROR_OK:
        return "ERROR_OK";

        //  //////////////////////////////////////
        //  Receive data
        //  //////////////////////////////////////
    case ERROR_RECV_STATE_ERROR:
        return "ERROR_RECV_STATE_ERROR";

    case ERROR_RECV_STATE_TIMEOUT:
        return "ERROR_RECV_STATE_TIMEOUT";

    case ERROR_RECV_STATE_CLIENT_CLOSED:
        return "ERROR_RECV_STATE_CLIENT_CLOSED";

    case ERROR_RECV_STATE_END_FLAG:
        return "ERROR_RECV_STATE_END_FLAG";

    case ERROR_RECV_STATE_CHECK_PACK_FAIL:
        return "ERROR_RECV_STATE_CHECK_PACK_FAIL";

    case ERROR_RECV_STATE_ERR_TOO_LONG_REQ:
        return "ERROR_RECV_STATE_ERR_TOO_LONG_REQ";

    case ERROR_RECV_STATE_END:
        return "ERROR_RECV_STATE_END";

        //  //////////////////////////////////////
    case ERROR_PROCESS_DOWNSTREAM_BUSY:
        return "ERROR_PROCESS_DOWNSTREAM_BUSY";

    case ERROR_PROCESS_EPOLL_FD_ERROR:
        return "ERROR_PROCESS_EPOLL_FD_ERROR";

    case ERROR_PROCESS_END:
        return "ERROR_PROCESS_END";

        //  //////////////////////////////////////
        //  Send data state
        //  //////////////////////////////////////
    case ERROR_SEND_STATE_ERROR:
        return "ERROR_SEND_STATE_ERROR";

    case ERROR_SEND_STATE_NETWORK_BUSY:
        return "ERROR_SEND_STATE_NETWORK_BUSY";

    case ERROR_SEND_STATE_TRY_AGAIN:
        return "ERROR_SEND_STATE_TRY_AGAIN";

    case ERROR_SEND_STATE_EWOULDBLOCK:
        return "ERROR_SEND_STATE_EWOULDBLOCK";

    case ERROR_SEND_STATE_FINISHED:
        return "ERROR_SEND_STATE_FINISHED";

    case ERROR_SEND_STATE_END:
        return "ERROR_SEND_STATE_END";

        //  //////////////////////////////////////
        //  CSocketIOThread_T
        //  //////////////////////////////////////

        //  Group task error codes
    case GERR_ALLFAIL_TIMEOUT:
        return "GERR_ALLFAIL_TIMEOUT";

    case GERR_PART_SUCCESS:
        return "GERR_PART_SUCCESS";

    case GERR_FULL_SUCCESS:
        return "GERR_FULL_SUCCESS";

        //  //////////////////////////////////////
    case ERROR_SOCKETIO_ADD_NODE_TO_TIMED_LIST_FAIL:
        return "ERROR_SOCKETIO_ADD_NODE_TO_TIMED_LIST_FAIL";

    case ERROR_SOCKETIO_RECV_REQUEST_TIMEOUT:
        return "ERROR_SOCKETIO_RECV_REQUEST_TIMEOUT";

    case ERROR_SOCKETIO_SEND_RESPONSE_TIMEOUT:
        return "ERROR_SOCKETIO_SEND_RESPONSE_TIMEOUT";

    case ERROR_SOCKETIO_SEND_REQUEST_TIMEOUT:
        return "ERROR_SOCKETIO_SEND_REQUEST_TIMEOUT";

    case ERROR_SOCKETIO_RECV_RESPONSE_TIMEOUT:
        return "ERROR_SOCKETIO_RECV_RESPONSE_TIMEOUT";

    case ERROR_SOCKETIO_INVALID_REQUEST_PACKLEN:
        return "ERROR_SOCKETIO_INVALID_REQUEST_PACKLEN";

    case ERROR_SOCKETIO_FORCE_END_ALL_SESSIONS:
        return "ERROR_SOCKETIO_FORCE_END_ALL_SESSIONS";

    case ERROR_SOCKETIO_ADD_TO_EPOLL_FAIL:
        return "ERROR_SOCKETIO_ADD_TO_EPOLL_FAIL";

        // ...
    case ERROR_SOCKETIO_END:
        return "ERROR_SOCKETIO_END";

        //  //////////////////////////////////////
        //  Business error code
    case ERROR_BUSINESS_PREPARE_FD_TO_DOWNSTREAM_FAIL:
        return "ERROR_BUSINESS_PREPARE_FD_TO_DOWNSTREAM_FAIL";

        // ...
    case ERROR_BUSINESS_END:
        return "ERROR_BUSINESS_END";

    case ERROR_SVRPUB_END:
        return "ERROR_SVRPUB_END";
        //
    default:
        return "unknown error code.";
    }
}

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_ERRORCODE_SVRPUBLIB_H_
