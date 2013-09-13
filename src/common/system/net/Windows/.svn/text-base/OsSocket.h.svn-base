#ifndef OS_SOCKET_HPP_INCLUDED
#define OS_SOCKET_HPP_INCLUDED

#include <winsock2.h>
#include <ws2tcpip.h>

#define CONFIG_IPV6 1

typedef unsigned short sa_family_t;
typedef unsigned int in_addr_t;
typedef unsigned short in_port_t;
const SOCKET INVALID_SOCKET_HANDLE = INVALID_SOCKET;
const int SOCKET_ERROR_RETURN = SOCKET_ERROR;

inline int SocketGetLastError()
{
    return WSAGetLastError();
}

inline void  SocketSetLastError(int error)
{
    WSASetLastError(error);
}

inline std::string SocketGetErrorString(int error)
{
    std::string result;
    void* msg_buf;
    if (FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msg_buf,
        0,
        NULL
    ))
    {
        result = (char*)msg_buf;
        LocalFree(msg_buf);
    }
    return result;
}

inline int SocketShutdown(SOCKET s)
{
    return shutdown(s, SD_SEND);
}

inline int SocketShutdownSend(SOCKET s)
{
    return shutdown(s, SD_RECEIVE);
}

inline int SocketShutdownReceive(SOCKET s)
{
    return shutdown(s, SD_BOTH);
}

inline int SocketSetNonblocking(SOCKET s, bool value)
{
    u_long option_value = value;
    return ioctlsocket(s, FIONBIO, &option_value);
}

inline int SocketGetNonblocking(SOCKET s, bool& value)
{
    return -1;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127)
#endif

inline int Socket_PollReadable(SOCKET fd, struct timeval* tv)
{
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fd + 1, &fdset);
    return select((int)(fd+1), &fdset, NULL, NULL, tv);
}

inline int Socket_PollWriteable(SOCKET fd, struct timeval* tv)
{
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fd + 1, &fdset);
    return select((int)(fd+1), NULL, &fdset, NULL, tv);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define SOCKET_ERROR_CODE(e) WSA##e

#pragma comment(lib, "ws2_32")
//WSAStringToAddress

#endif//OS_SOCKET_HPP_INCLUDED
