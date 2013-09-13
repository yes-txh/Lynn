// --------------------------------------------------
// general_head.h
// wookin
// --------------------------------------------------

#ifndef COMMON_BASELIB_SVRPUBLIB_GENERAL_HEAD_H_
#define COMMON_BASELIB_SVRPUBLIB_GENERAL_HEAD_H_

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#include <signal.h>


#ifdef WIN32

// force use winsock2.h
#ifndef _WINSOCK2_ENABLE
#define _WINSOCK2_ENABLE
#endif

#ifdef _WINSOCK2_ENABLE
#include <winsock2.h>
#else
#include <winsock.h>
#endif //

#include <windows.h>
#include <time.h>
#include <tchar.h>
#include <process.h>

//  IO
#include <sys/stat.h>
#include <io.h>

//  c++
#include <typeinfo.h>

#else // linux
//  IO
#include <stdlib.h>
#include <stdarg.h>
#include <aio.h>

//  Socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
// #include <fcntl.h>
#include <netdb.h>

//  Real epoll support
#ifndef _FAKE_EPOLL
#include <sys/epoll.h>
#endif // !_FAKE_EPOLL

//  Types
#include <ctype.h>

// For uname
#include <sys/utsname.h>

//  Time
#include <sys/time.h>
#include <unistd.h>

//  Thread
#include <pthread.h>

//  shm
#include <sys/ipc.h>
#include <sys/shm.h>

//  Semphore
#include <semaphore.h>

//  Resource
#include <sys/resource.h>

//  C++
#include <typeinfo>

#endif // linux

#endif // COMMON_BASELIB_SVRPUBLIB_GENERAL_HEAD_H_

