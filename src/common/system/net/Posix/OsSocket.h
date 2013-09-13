#ifndef OS_SOCKET_HPP_INCLUDED
#define OS_SOCKET_HPP_INCLUDED

#ifdef __CYGWIN__
#define CONFIG_IPV6 0
#else
#define CONFIG_IPV6 1
#endif

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

typedef int SOCKET;

const SOCKET INVALID_SOCKET_HANDLE = -1;
const int SOCKET_ERROR_RETURN = -1;

inline int SocketGetLastError()
{
	return errno;
}

inline void SocketSetLastError(int error)
{
	errno = error;
}

inline std::string SocketGetErrorString(int error)
{
	return strerror(error);
}

inline int closesocket(SOCKET s)
{
	return close(s);
}

inline int ioctlsocket(SOCKET s, int cmd, void* p)
{
	return ioctl(s, cmd, p);
}

inline int SocketShutdown(SOCKET s)
{
	return shutdown(s, SHUT_RDWR);
}

inline int SocketShutdownSend(SOCKET s)
{
	return shutdown(s, SHUT_WR);
}

inline int SocketShutdownReceive(SOCKET s)
{
	return shutdown(s, SHUT_RD);
}

inline int SocketSetNonblocking(SOCKET s, bool value)
{
	int flags = fcntl(s, F_GETFL, 0);
	if (flags < 0)
		return flags;
	if (value)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	return fcntl(s, F_SETFL, flags);
}

inline int SocketGetNonblocking(SOCKET s, bool& value)
{
	int flags = fcntl(s, F_GETFL, 0);
	if (flags < 0)
		return flags;
	value = (flags & O_NONBLOCK) != 0;
	return true;
}

inline int Socket_PollReadable(SOCKET fd, struct timeval* tv)
{
	int timeout = tv ? (tv->tv_sec * 1000 + tv->tv_usec / 1000) : -1;
	pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	return poll(&pfd, 1, timeout);
}

inline int Socket_PollWriteable(SOCKET fd, struct timeval* tv)
{
	int timeout = tv ? (tv->tv_sec * 1000 + tv->tv_usec / 1000) : -1;
	pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLOUT;
	return poll(&pfd, 1, timeout);
}


#define SOCKET_ERROR_CODE(e) e

#endif//OS_SOCKET_HPP_INCLUDED
