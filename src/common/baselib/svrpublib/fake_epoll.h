//  fake EPOLL win32 use only
//  fake_epoll.h
//
//  wookin@tencent.com
//  2007.06.21
//
//  1:BoundsChecker不认识从int32_t* 到int32_t的跟踪，
//    所以我将Epoll handle从int修改为void*
//
//  2:在linux下请将EpollHandle从int32_t替换为EPOLLHANDLE
//
// //////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_FAKE_EPOLL_H_
#define COMMON_BASELIB_SVRPUBLIB_FAKE_EPOLL_H_

#include "common/baselib/svrpublib/general_sock.h"
#include "common/baselib/svrpublib/base_config.h"

//
//  Auto enable fake epoll
//
#ifdef WIN32
#define _FAKE_EPOLL
#endif

#if defined(_FAKE_EPOLL)

//  /////////////////////////////////////
//  Fake EPOLL
//  /////////////////////////////////////

//
//  EPOLL EVENTS
//
enum EPOLL_EVENTS {
    EPOLLIN = 0x001,
#define EPOLLIN EPOLLIN
    EPOLLPRI = 0x002,
#define EPOLLPRI EPOLLPRI
    EPOLLOUT = 0x004,
#define EPOLLOUT EPOLLOUT
    EPOLLRDNORM = 0x040,
#define EPOLLRDNORM EPOLLRDNORM
    EPOLLRDBAND = 0x080,
#define EPOLLRDBAND EPOLLRDBAND
    EPOLLWRNORM = 0x100,
#define EPOLLWRNORM EPOLLWRNORM
    EPOLLWRBAND = 0x200,
#define EPOLLWRBAND EPOLLWRBAND
    EPOLLMSG = 0x400,
#define EPOLLMSG EPOLLMSG
    EPOLLERR = 0x008,
#define EPOLLERR EPOLLERR
    EPOLLHUP = 0x010,
#define EPOLLHUP EPOLLHUP
    EPOLLONESHOT = (1 << 30),
#define EPOLLONESHOT EPOLLONESHOT
    EPOLLET = (1 << 31)
#define EPOLLET EPOLLET
};

// Valid opcodes ( "op" parameter ) to issue to epoll_ctl().
#define EPOLL_CTL_ADD 1 // Add a file decriptor to the interface.
#define EPOLL_CTL_DEL 2 // Remove a file decriptor from the interface.
#define EPOLL_CTL_MOD 3 // Change file decriptor epoll_event structure.

union epoll_data_t {
    void*          ptr;
    SOCKET         fd;
#ifdef WIN32
    __int32     u32;
    __int64     u64;
#else // linux
    __uint32_t  u32;
    __uint64_t  u64;
#endif // linux
};

struct epoll_event {
#ifdef WIN32
    int32_t     events; //  Epoll events
#else // linux
    __uint32_t  events; //  Epoll events
#endif // linux
    epoll_data_t    data;   //  User data variable
};

//
//  Fake EPOLL event
//
struct FakeEpollEvent {
    struct epoll_event  epollevent;
    SOCKET              sockfd;
};

struct FakeEpollNode {
    FakeEpollEvent          node;
    struct FakeEpollNode*   next;
};

struct FakeEPOLL_HANDLE {
    FakeEpollNode*  m_valid_head;       //  Valid epoll nodes queue
    FakeEpollNode*  m_valid_cursor;     //  Cursor pointer in valid nodes queue
    FakeEpollNode*  m_empty_head;       //  Empty epoll nodes queue
    unsigned char*  m_buffer;
};

_START_XFS_BASE_NAMESPACE_

bool    epoll_fd_exist(EPOLLHANDLE epfd, SOCKET fd);
int32_t get_valid_count(struct epoll_event * events, int32_t max_events);
void    combin_events(struct epoll_event * events, int32_t max_events,
                      epoll_data_t* data, int32_t num_event);

//
//  Fake EPOLL
//
EPOLLHANDLE fake_epoll_create(int32_t size);
int32_t     fake_epoll_close(EPOLLHANDLE epfd);
int32_t     fake_epoll_ctl(EPOLLHANDLE epfd, int32_t op, SOCKET fd,
                           const struct epoll_event* ev);
int32_t     fake_epoll_wait(EPOLLHANDLE epfd, struct epoll_event* events,
                            int32_t max_events, int32_t timeout);

//
//  epoll_fd_set
//
struct epoll_fd_set {
    fd_set          set;                    //  Real set
    epoll_data_t    data_array[FD_SETSIZE]; //  User data
    uint32_t        fd_array[FD_SETSIZE];   //  fd array
    uint32_t        num_fd_count;           //  Valid fd count

    epoll_fd_set() {
        memset(this, 0, sizeof(struct epoll_fd_set));
    }
};

void epoll_FD_SET(uint32_t fd, epoll_fd_set* pset, epoll_data_t user_data);

//  defined(_FAKE_EPOLL)
#define epoll_create    fake_epoll_create
#define epoll_ctl       fake_epoll_ctl
#define epoll_wait      fake_epoll_wait
#define epoll_close     fake_epoll_close

_END_XFS_BASE_NAMESPACE_

//  /////////////////////////////////////
#endif // _FAKE_EPOLL
//  /////////////////////////////////////

#ifndef epoll_close
#define epoll_close close
#endif // epoll_close

//
//  End of head file
//
#endif // COMMON_BASELIB_SVRPUBLIB_FAKE_EPOLL_H_
