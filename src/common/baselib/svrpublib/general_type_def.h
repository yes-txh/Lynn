// general_type_def.h    general type and macro define
// wookin
// /////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_GENERAL_TYPE_DEF_H_
#define COMMON_BASELIB_SVRPUBLIB_GENERAL_TYPE_DEF_H_

#include "common/baselib/svrpublib/twse_type_def.h"
#include "common/baselib/svrpublib/base_config.h"

// /////////////////////////////////
//  START: public define
// /////////////////////////////////

// typedef THANDLE
#ifndef THANDLE
typedef void*               THANDLE;
#endif // THANDLE

#define INVALID_FD          -1

#ifdef WIN32 // //////////////////////////
typedef uint32_t        in_addr_t;
typedef void*           EPOLLHANDLE;
typedef int32_t         socklen_t;
// 需要考虑m64
#define INVALID_HANDLE  (THANDLE)(-1)

// thread_t
typedef HANDLE      THREAD_T;

#else // linux    ///////////////////////
typedef int32_t         EPOLLHANDLE;

// 需要考虑m64
#define INVALID_HANDLE  (-1)

// /////////////////////////////////
#ifndef SOCKET
typedef int32_t     SOCKET;
#endif // SOCKET

// /////////////////////////////////

// /////////////////////////////////
// START: public define on Linux OS
// /////////////////////////////////

//  C++ interface
#ifndef interface
#define interface           class
#endif // interface

//  Socket
typedef int SOCKET;
const SOCKET INVALID_SOCKET = -1;

//#define INVALID_SOCKET      -1          //  winsock.h[WIN32]
#define MAX_PATH            256
#define INFINITE            0xFFFFFFFF  //  Infinite timeout

typedef pthread_t   THREAD_T;


// /////////////////////////////////
// END: public define on Linux OS
// /////////////////////////////////
#endif


// /////////////////////////////////
// START: public define on WIN32 OS
// /////////////////////////////////

#ifdef WIN32
#if defined(__cplusplus) && !defined(CINTERFACE)
//  C++ interface
#ifndef interface
#define interface           class
#endif // interface
#endif // __cplusplus && CINTERFACE

//  Socket
#ifndef _WINSOCKAPI_
#pragma message("<*General Lib WARNING*>if you want to use "
"SD_BOTH or SD_RECEIVE or SD_SEND, "
"you must include <Winsock2.h> first.")
#endif // _WINSOCKAPI_

// shutdown macro
#define SHUT_RDWR    SD_BOTH
#define SHUT_RD      SD_RECEIVE
#define SHUT_WR      SD_SEND
#endif // WIN32
// /////////////////////////////////
//  END: public define on WIN32 OS
// /////////////////////////////////

// /////////////////////////////////
// START: public define and MACRO
// /////////////////////////////////
// #ifndef FILENAME_BUFF
// typedef    char    FILENAME_BUFF[MAX_PATH];
// #endif // FILENAME_BUFF

_START_XFS_BASE_NAMESPACE_

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif //


#ifndef XGUID_DEF
#define XGUID_DEF
struct XGUID {
    uint32_t        data1;
    uint16_t        data2;
    uint16_t        data3;
    unsigned char   data4[8];

    void ToNetOrder() {
        data1 = htonl(data1);
        data2 = htons(data2);
        data3 = htons(data3);
    }

    void ToHostOrder() {
        data1 = ntohl(data1);
        data2 = ntohs(data2);
        data3 = ntohs(data3);
    }

    XGUID() {
        memset(this, 0, sizeof(XGUID));
    }

    XGUID(const XGUID& other) {
        memcpy(this, &other, sizeof(XGUID));
    }
    
    XGUID& operator = (const XGUID& other) {
        memcpy(this, &other, sizeof(XGUID));
        return *this;
    }

    bool operator == (const XGUID& other) const {
        return memcmp(this, &other, sizeof(XGUID)) == 0;
    }

    bool operator < (const XGUID& other) const {
        return memcmp(this, &other, sizeof(XGUID)) < 0;
    }

    void Empty() {
        memset(this, 0, sizeof(XGUID));
    }
};
#define XIID    XGUID
#define XCLSID  XGUID
#endif // XGUID_DEF

#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif //


enum TCP_FD_TYPE {
    TCP_FD_UNKNOWN = 0,
    TCP_FD_NEW,
    TCP_FD_ACCEPTED,
};


# ifndef POSIX_EWOULDBLOCK
#  ifdef WIN32
#   define POSIX_EWOULDBLOCK WSAEWOULDBLOCK
#  else
#   define POSIX_EWOULDBLOCK EWOULDBLOCK
#  endif
# endif

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_GENERAL_TYPE_DEF_H_

