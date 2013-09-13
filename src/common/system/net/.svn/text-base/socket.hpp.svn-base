#ifndef SYSTEM_NET_SOCKET_HPP
#define SYSTEM_NET_SOCKET_HPP

#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <stdexcept>
#include <string>
#include <utility>
#include "common/base/stdint.h"
#include "common/system/net/OsSocket.h"

/// socket error exception class
class SocketError : public std::runtime_error
{
public:
    SocketError(const char* info = "socket error", int error = SocketGetLastError()):
        std::runtime_error(std::string(info) + std::string(": ") + SocketGetErrorString(error)),
        m_error(error)
    {
    }

    int Code() const
    {
        return m_error;
    }
private:
    int m_error;
};

/// Abstract sockaddr
class SocketAddress
{
protected:
    SocketAddress(){}
    virtual ~SocketAddress(){}
public:
     /// get address family
    virtual sa_family_t Family() const = 0;

    /// get as sockaddr*
    sockaddr* Address()
    {
        return const_cast<sockaddr*>(DoGetAddress());
    }

    /// get as sockaddr*
    const sockaddr* Address() const
    {
        return DoGetAddress();
    }

    /// get address length
    virtual socklen_t Length() const = 0;

    /// set address length
    virtual bool SetLength(socklen_t length) = 0;

    /// set max capacity, for variable length address
    virtual socklen_t Capacity() const = 0;

    /// convert to string
    void ToString(std::string& str) const
    {
        return DoToString(str);
    }

    std::string ToString() const
    {
        std::string str;
        DoToString(str);
        return str;
    }

    bool Parse(const char* str)
    {
        return DoParse(str);
    }
    bool Parse(const std::string& str)
    {
        return DoParse(str.c_str());
    }

    bool CopyFrom(const SocketAddress& rhs)
    {
        return DoCopyFrom(rhs);
    }

    SocketAddress& operator=(const SocketAddress& rhs)
    {
        if (!CopyFrom(rhs))
            throw std::runtime_error("SocketAddress: Can't copy from " + rhs.ToString());
        return *this;
    }
private:
    virtual bool DoParse(const char* str) = 0;
    virtual const sockaddr* DoGetAddress() const = 0;
    virtual void DoToString(std::string& str) const = 0;
    virtual bool DoCopyFrom(const SocketAddress& rhs) = 0;
};

/// any sockaddr type encapsulation
template <typename Type, sa_family_t AF>
class SocketAddressTemplate : public SocketAddress
{
protected:
    SocketAddressTemplate() : m_address()
    {
        reinterpret_cast<sockaddr&>(m_address).sa_family = AF;
    }
public:
    virtual sa_family_t Family() const
    {
        return m_generic_address.sa_family;
    }
    virtual socklen_t Length() const
    {
        return sizeof(m_address);
    }
    virtual bool SetLength(socklen_t length)
    {
        return length == sizeof(m_address);
    }
    socklen_t Capacity() const
    {
        return sizeof(m_address);
    }
private:
    virtual const sockaddr* DoGetAddress() const
    {
        return &m_generic_address;
    }
    virtual bool DoCopyFrom(const SocketAddress& rhs)
    {
        if (rhs.Family() != Family())
            return false;
        memcpy(this->Address(), rhs.Address(), rhs.Length());
        return true;
    }
protected:
    union
    {
        Type m_address;
        sockaddr m_generic_address;
#ifdef _WIN32 // sockaddr_storage 的对齐依赖于头文件包含顺序，取到最大值
        unsigned __int64 align;
#endif
    };
};

class IPAddress
{
    typedef unsigned char BytesType[4];
public:
    IPAddress() : m_ip()
    {
    }
    explicit IPAddress(const char* src)
    {
        if (!Assign(src))
            throw std::runtime_error(std::string("Invalid IP Address: ") + src);
    }
    explicit IPAddress(const std::string& src)
    {
        if (!Assign(src))
            throw std::runtime_error("Invalid IP Address: " + src);
    }
    explicit IPAddress(unsigned int ip)
    {
        Assign(ip);
    }

    IPAddress(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4)
    {
        Assign(b1, b2, b3, b4);
    }

    explicit IPAddress(in_addr addr)
    {
        Assign(addr);
    }

public:
    void Assign(unsigned int ip) // network byte order
    {
        m_ip.s_addr = ip;
    }

    void Assign(in_addr addr)
    {
        m_ip = addr;
    }

    void Assign(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4)
    {
        bytes[0] = b1;
        bytes[1] = b2;
        bytes[2] = b3;
        bytes[3] = b4;
    }

    bool Assign(const char* src)
    {
        unsigned int b1, b2, b3, b4;
        char dummy; // catch extra character
        int count = sscanf(src, "%u.%u.%u.%u%c", &b1, &b2, &b3, &b4, &dummy);
        if (count == 4 && b1 <= UCHAR_MAX && b2 <= UCHAR_MAX && b3 <= UCHAR_MAX && b4 <= UCHAR_MAX)
        {
            Assign((unsigned char)b1, (unsigned char)b2, (unsigned char)b3, (unsigned char)b4);
            return true;
        }
        return false;
    }
    bool Assign(const std::string& src)
    {
        return Assign(src.c_str());
    }
    in_addr_t ToInt() const
    {
        return m_ip.s_addr;
    }
    in_addr_t ToLocalInt() const
    {
        return ntohl(m_ip.s_addr);
    }
    const in_addr ToInAddr() const
    {
        return m_ip;
    }
    void ToString(std::string& str) const
    {
        char text[INET_ADDRSTRLEN];
        int length = sprintf(text, "%u.%u.%u.%u", bytes[0], bytes[1], bytes[2], bytes[3]);
        str.assign(text, length);
    }
    const std::string ToString() const
    {
        std::string result;
        ToString(result);
        return result;
    }
    bool IsLoopback() const
    {
        return bytes[0] == 127;
    }
    bool IsBroadcast() const
    {
        return bytes[3] == 255;
    }
    bool IsPrivate() const
    {
        return bytes[0] == 10 ||
            (bytes[0] == 172 && bytes[1] >= 16 && bytes[1] <= 31) ||
            (bytes[0] == 192 && bytes[1] == 168);
    }
    const BytesType& Bytes() const
    {
        return bytes;
    }
public:
    static const IPAddress None()
    {
        return IPAddress(htonl(INADDR_NONE));
    }
    static const IPAddress Any()
    {
        return IPAddress(htonl(INADDR_ANY));
    }
    static const IPAddress Broadcast()
    {
        return IPAddress(htonl(INADDR_BROADCAST));
    }
    static const IPAddress Loopback()
    {
        return IPAddress(htonl(INADDR_LOOPBACK));
    }
private:
    union
    {
        in_addr m_ip;
        unsigned char bytes[4];
    };
};

inline bool operator==(const IPAddress& lhs, const IPAddress& rhs)
{
    return lhs.ToInt() == rhs.ToInt();
}

inline bool operator!=(const IPAddress& lhs, const IPAddress& rhs)
{
    return lhs.ToInt() != rhs.ToInt();
}

inline bool operator<(const IPAddress& lhs, const IPAddress& rhs)
{
    return lhs.ToLocalInt() < rhs.ToLocalInt();
}

inline bool operator>(const IPAddress& lhs, const IPAddress& rhs)
{
    return lhs.ToLocalInt() > rhs.ToLocalInt();
}

inline bool operator<=(const IPAddress& lhs, const IPAddress& rhs)
{
    return lhs.ToLocalInt() <= rhs.ToLocalInt();
}

inline bool operator>=(const IPAddress& lhs, const IPAddress& rhs)
{
    return lhs.ToLocalInt() >= rhs.ToLocalInt();
}

/// Socket address encapsulation for IPv4
class SocketAddressInet4 : public SocketAddressTemplate<sockaddr_in, AF_INET>
{
    typedef SocketAddressTemplate<sockaddr_in, AF_INET> Base;
public:
    SocketAddressInet4()
    {
    }

    SocketAddressInet4(unsigned int ip, unsigned short port)
    {
        Assign(ip, port);
    }

    SocketAddressInet4(const IPAddress& ip, unsigned short port)
    {
        Assign(ip, port);
    }

    SocketAddressInet4(
        unsigned char b1,
        unsigned char b2,
        unsigned char b3,
        unsigned char b4,
        unsigned short port
    )
    {
        Assign(b1, b2, b3, b4, port);
    }

    explicit SocketAddressInet4(const char* src)
    {
        if (!Assign(src))
        {
            throw std::runtime_error(std::string("Invalid IPv4 socket address: ") + src);
        }
    }

    explicit SocketAddressInet4(const std::string& src)
    {
        if (!Assign(src))
        {
            throw std::runtime_error(std::string("invalid ipv4 socket address: ") + src);
        }
    }

    SocketAddressInet4(const char* src, unsigned short port)
    {
        if (!Assign(src, port))
        {
            throw std::runtime_error(std::string("Invalid IPv4 socket address: ") + src);
        }
    }

    SocketAddressInet4(const std::string& src, unsigned short port)
    {
        if (!Assign(src, port))
        {
            throw std::runtime_error(std::string("Invalid IPv4 socket address: ") + src);
        }
    }

    bool Assign(unsigned int ip, unsigned short port)
    {
        m_address.sin_addr.s_addr = ip;
        m_address.sin_port = htons(port);
        return true;
    }

    bool Assign(const IPAddress& ip, unsigned short port)
    {
        m_address.sin_addr = ip.ToInAddr();
        m_address.sin_port = htons(port);
        return true;
    }

    bool Assign(
        unsigned char b1,
        unsigned char b2,
        unsigned char b3,
        unsigned char b4,
        unsigned short port
        )
    {
        unsigned char* p = reinterpret_cast<unsigned char*>(&m_address.sin_addr);
        p[0] = b1;
        p[1] = b2;
        p[2] = b3;
        p[3] = b4;

        m_address.sin_port = htons(port);

        return true;
    }

    bool Assign(const char* str)
    {
        unsigned int b1, b2, b3, b4;
        int port;
        char dummy; // catch extra character
        int count = sscanf(str, "%u.%u.%u.%u:%d%c", &b1, &b2, &b3, &b4, &port, &dummy);
        if (count == 5 &&
            b1 < UCHAR_MAX && b2 < UCHAR_MAX && b3 < UCHAR_MAX && b4 < UCHAR_MAX &&
            port >=0 && port < USHRT_MAX
            )
        {
            return Assign(
                (unsigned char)b1,
                (unsigned char)b2,
                (unsigned char)b3,
                (unsigned char)b4,
                port
            );
        }
        return false;
    }

    bool Assign(const char* str, unsigned short port)
    {
        IPAddress ip;
        if (ip.Assign(str))
            return Assign(ip, port);
        return false;
    }

    bool Assign(const std::string& ip, unsigned short port)
    {
        return Assign(ip.c_str(), port);
    }

    bool Assign(const std::string& src)
    {
        return Assign(src.c_str());
    }

    void SetIP(const IPAddress& address)
    {
        m_address.sin_addr = address.ToInAddr();
    }
    const IPAddress GetIP() const
    {
        return IPAddress(m_address.sin_addr);
    }

    unsigned short GetPort() const
    {
        return ntohs(m_address.sin_port);
    }
    void SetPort(uint16_t port)
    {
        m_address.sin_port = htons(port);
    }

    int Compare(const SocketAddressInet4& rhs) const
    {
        if (GetIP() > rhs.GetIP())
            return 1;
        else if (GetIP() < rhs.GetIP())
            return -1;

        return GetPort() - rhs.GetPort();
    }

private:
    void DoToString(std::string& str) const
    {
        char text[INET_ADDRSTRLEN + sizeof(":65536")];
        const unsigned char* p = reinterpret_cast<const unsigned char*>(&m_address.sin_addr);
        int length = sprintf(
            text, "%u.%u.%u.%u:%d",
            p[0], p[1], p[2], p[3], ntohs(m_address.sin_port)
        );
        str.assign(text, length);
    }

    virtual bool DoParse(const char* src)
    {
        return Assign(src);
    }
};

inline bool operator==(const SocketAddressInet4& lhs, const SocketAddressInet4& rhs)
{
    return lhs.Compare(rhs) == 0;
}

inline bool operator!=(const SocketAddressInet4& lhs, const SocketAddressInet4& rhs)
{
    return !(lhs == rhs);
}

inline bool operator<(const SocketAddressInet4& lhs, const SocketAddressInet4& rhs)
{
    return lhs.Compare(rhs) < 0;
}

inline bool operator<=(const SocketAddressInet4& lhs, const SocketAddressInet4& rhs)
{
    return lhs.Compare(rhs) <= 0;
}

inline bool operator>(const SocketAddressInet4& lhs, const SocketAddressInet4& rhs)
{
    return lhs.Compare(rhs) > 0;
}

inline bool operator>=(const SocketAddressInet4& lhs, const SocketAddressInet4& rhs)
{
    return lhs.Compare(rhs) >= 0;
}

#if CONFIG_IPV6
/// Socket address encapsulation for IPv6
class SocketAddressInet6 : public SocketAddressTemplate<sockaddr_in6, AF_INET6>
{
public:
    SocketAddressInet6()
    {
    }

    explicit SocketAddressInet6(const char* src)
    {
        if (!Parse(src))
        {
            throw std::runtime_error(std::string("Invalid IPv6 socket address: ") + src);
        }
    }

    explicit SocketAddressInet6(const std::string& src)
    {
        if (!Parse(src))
        {
            throw std::runtime_error(std::string("Invalid IPv6 socket address: ") + src);
        }
    }
private:
    virtual void ToString(std::string& str) const
    {
        str.clear();
    }
    virtual bool DoParse(const char* str)
    {
        return false;
    }
};
#endif

class SocketAddressInet : public SocketAddress
{
public:
    SocketAddressInet() : m_address()
    {
        m_address.common.sa_family = AF_UNSPEC;
    }

    explicit SocketAddressInet(const char* src) : m_address()
    {
        if (!Parse(src))
        {
            throw std::runtime_error(std::string("Invalid IPv4/6 socket address: ") + src);
        }
    }

    explicit SocketAddressInet(const std::string& src) : m_address()
    {
        if (!Parse(src))
        {
            throw std::runtime_error(std::string("Invalid IPv4/6 socket address: ") + src);
        }
    }

    /// @param ip 网络字节序
    /// @param port 本机字节序
    SocketAddressInet(unsigned int ip, unsigned short port) : m_address()
    {
        m_address.v4.sin_family = AF_INET;
        m_address.v4.sin_addr.s_addr = ip;
        m_address.v4.sin_port = htons(port);
    }

    SocketAddressInet(
        unsigned char b1,
        unsigned char b2,
        unsigned char b3,
        unsigned char b4,
        unsigned short port
    ) :
        m_address()
    {
        m_address.v4.sin_family = AF_INET;

        unsigned char* p = reinterpret_cast<unsigned char*>(&m_address.v4.sin_addr);
        p[0] = b1;
        p[1] = b2;
        p[2] = b3;
        p[3] = b4;

        m_address.v4.sin_port = htons(port);
    }

    virtual sa_family_t Family() const
    {
        return m_address.common.sa_family;
    }

    virtual socklen_t Length() const
    {
        switch (m_address.common.sa_family)
        {
        case AF_INET:
            return sizeof(m_address.v4);
        #if CONFIG_IPV6
        case AF_INET6:
            return sizeof(m_address.v6);
        #endif
        default:
            assert(false);
        }
        return false;
    }

    virtual bool SetLength(socklen_t length)
    {
        switch (m_address.common.sa_family)
        {
        case AF_INET:
            return length == sizeof(m_address.v4);
        #if CONFIG_IPV6
        case AF_INET6:
            return length == sizeof(m_address.v6);
        #endif
        default:
            assert(false);
        }
        return false;
    }

    virtual socklen_t Capacity() const
    {
        return sizeof(m_address);
    }

    SocketAddressInet& operator=(const SocketAddress& src)
    {
        if (src.Family() != AF_INET && src.Family() != AF_INET6) {
            throw std::runtime_error(
                "SocketAddress: Can't copy from " + src.ToString());
        }
        memcpy(Address(), src.Address(), src.Length());
        return *this;
    }

private:
    virtual const sockaddr* DoGetAddress() const
    {
        return &m_address.common;
    }

    virtual void DoToString(std::string& str) const
    {
        char buf[128];
        int length = 0;

        switch (m_address.common.sa_family)
        {
        case AF_INET:
            {
                const unsigned char* p =
                    reinterpret_cast<const unsigned char*>(&m_address.v4.sin_addr);
                length = sprintf(
                    buf,
                    "%d.%d.%d.%d:%d", p[0], p[1], p[2], p[3],
                    ntohs(m_address.v4.sin_port)
                );
            }
            break;
        #if CONFIG_IPV6
        case AF_INET6:
            break;
        #endif
        default:
            assert(false);
            break;
        }
        str.assign(buf, length);
    }
    virtual bool DoParse(const char* str)
    {
        unsigned int b1, b2, b3, b4;
        int port;
        char dummy; // catch extra character
        int count = sscanf(str, "%u.%u.%u.%u:%d%c", &b1, &b2, &b3, &b4, &port, &dummy);
        if (count == 5 &&
            b1 <= UCHAR_MAX && b2 <= UCHAR_MAX && b3 <= UCHAR_MAX && b4 <= UCHAR_MAX &&
            port >= 0 && port <= USHRT_MAX
            )
        {
            m_address.v4.sin_family = AF_INET;

            unsigned char* p =
                reinterpret_cast<unsigned char*>(&m_address.v4.sin_addr);

            p[0] = (unsigned char) b1;
            p[1] = (unsigned char) b2;
            p[2] = (unsigned char) b3;
            p[3] = (unsigned char) b4;

            m_address.v4.sin_port = htons(port);

            return true;
        }

        return false;
    }
    virtual bool DoCopyFrom(const SocketAddress& rhs)
    {
        switch (rhs.Family())
        {
        case AF_INET:
        case AF_INET6:
        default:
            return false;
        }
    }
private:
    union
    {
        sockaddr common;
        sockaddr_in v4;
    #if CONFIG_IPV6
        sockaddr_in6 v6;
    #endif
    } m_address;
};

#ifdef __unix__
/// Unix domain socket address
class SocketAddressUnix : public SocketAddressTemplate<sockaddr_un, AF_UNIX>
{
public:
    SocketAddressUnix()
    {
    }
    explicit SocketAddressUnix(const char* name)
    {
        if (!Parse(name))
            throw std::runtime_error(std::string("Invalid unix domain socket address: ") + name);
    }
    explicit SocketAddressUnix(const std::string& name)
    {
        if (!Parse(name))
            throw std::runtime_error(std::string("Invalid unix domain socket address: ") + name);
    }
    virtual socklen_t Length() const
    {
        return offsetof(struct sockaddr_un, sun_path) + strlen(m_address.sun_path);
    }
private:
    virtual void DoToString(std::string& str) const
    {
        str.assign(m_address.sun_path);
    }
    virtual bool DoParse(const char* name)
    {
        // "/data/local.socket"
        // "@/data/local.socket"
        // check invalid path
        if (name[0] == '/' || (name[0] == '@' && name[1] == '/'))
        {
            size_t length = strlen(name);
            if (length + 1> sizeof(m_address.sun_path))
                return false;
            memcpy(m_address.sun_path, name, length + 1);
            return true;
        }
        return false;
    }
};

typedef SocketAddressUnix SocketAddressLocal;

#endif // __unix__

/// for store any type socket address
class SocketAddressStorage : public SocketAddressTemplate<sockaddr_storage, AF_UNSPEC>
{
public:
    SocketAddressStorage() : m_length(0)
    {
    }
    explicit SocketAddressStorage(const SocketAddress& src)
    {
        memcpy(Address(), src.Address(), src.Length());
        m_length = src.Length();
    }
    SocketAddressStorage& operator=(const SocketAddress& src)
    {
        memcpy(Address(), src.Address(), src.Length());
        m_length = src.Length();
        return *this;
    }
    virtual sa_family_t Family() const
    {
        return m_address.ss_family;
    }
    virtual socklen_t Length() const
    {
        return m_length;
    }
    virtual bool SetLength(socklen_t length)
    {
        if (length > (socklen_t)sizeof(m_address))
            return false;
        m_length = length;
        return true;
    }

private:
    virtual const sockaddr* DoGetAddress() const
    {
        return &m_generic_address;
    }
    virtual void DoToString(std::string& str) const
    {
        str.clear();
        switch (m_address.ss_family)
        {
        case AF_UNIX:
#ifdef unix
            str = reinterpret_cast<const sockaddr_un&>(m_address).sun_path;
#endif
            break;
        case AF_INET:
            {
                const sockaddr_in* saddrin = reinterpret_cast<const sockaddr_in*>(&m_address);
                char buffer[64];
                const unsigned char* p =
                    reinterpret_cast<const unsigned char*>(&saddrin->sin_addr);
                int length = sprintf(
                    buffer,
                    "%d.%d.%d.%d:%d", p[0], p[1], p[2], p[3],
                    ntohs(saddrin->sin_port)
                );
                str.assign(buffer, length);
            }
            break;
#if CONFIG_IPV6
        case AF_INET6:
            break;
#endif
        default:
            break;
        };
    }
    virtual bool DoParse(const char* str)
    {
        // try inet address
        {
            SocketAddressInet address;
            if (address.Parse(str))
            {
                *this = address;
                return true;
            }
        }
#ifdef __unix__
        // try unix domain address
        {
            SocketAddressUnix address;
            if (address.Parse(str))
            {
                *this = address;
                return true;
            }
        }
#endif
        return false;
    }
    virtual bool DoCopyFrom(const SocketAddress& rhs)
    {
        m_address.ss_family = rhs.Family();
        memcpy(this->Address(), rhs.Address(), rhs.Length());
        return SetLength(rhs.Length());
    }
private:
    socklen_t m_length;
};

/// Abstract socket base class
class Socket
{
public:
    static const SOCKET InvalidHandle = INVALID_SOCKET_HANDLE;
protected:
    Socket() : m_handle(InvalidHandle), m_ExceptionEnabled(false)
    {
    }
    explicit Socket(SOCKET handle) : m_handle(handle), m_ExceptionEnabled(false)
    {
    }
    bool Create(int af, int type, int protocol = 0)
    {
        Close();
        m_handle = socket(af, type, protocol);
        if (IsValid())
        {
            return true;
        }
        else
        {
            ReportError("Create");
            return false;
        }
    }
    Socket(const Socket&);
    Socket& operator=(const Socket&);

public:
    virtual ~Socket()
    {
        Close();
    }
    SOCKET Handle() const
    {
        return m_handle;
    }

    bool IsValid() const
    {
        return m_handle != InvalidHandle;
    }

    // Attach a socket handle to this object
    void Attach(SOCKET handle)
    {
        if (handle != m_handle)
        {
            closesocket(m_handle);
            m_handle = handle;
        }
    }

    // detach socket handle from this object
    SOCKET Detach()
    {
        SOCKET old = m_handle;
        m_handle = InvalidHandle;
        return old;
    }

    bool Close()
    {
        if (IsValid())
        {
            SOCKET fd = m_handle;
            m_handle = InvalidHandle;
            return CheckError(closesocket(fd), "Close");
        }
        return false;
    }

    /* Set the FD_CLOEXEC flag of desc if value is nonzero,
       or clear the flag if value is 0.
       Return 0 on success, or -1 on error with errno set. */

    static bool SetCloexec(int desc, bool value)
    {
#ifdef __unix__
        int oldflags = fcntl(desc, F_GETFD, 0);
        if (oldflags < 0)
            return false;

        if (value)
            oldflags |= FD_CLOEXEC;
        else
            oldflags &= ~FD_CLOEXEC;

        return fcntl(desc, F_SETFD, oldflags) >= 0;
#else
        return true;
#endif
    }

    bool SetCloexec(bool value = true)
    {
        return SetCloexec(m_handle, value);
    }

    void EnableException(bool value = true)
    {
        m_ExceptionEnabled = value;
    }

    bool ExtensionEnabled() const
    {
        return m_ExceptionEnabled;
    }

    bool GetOption(int level, int name, void* value, socklen_t& length) const
    {
        return CheckError(getsockopt(m_handle, level, name, (char*) value, &length), "GetOption");
    }

    bool SetOption(int level, int name, const void* value, socklen_t length)
    {
        return CheckError(setsockopt(m_handle, level, name, (char*) value, length), "SetOption");
    }

    template <typename T>
    bool GetOption(int level, int name, T& value) const
    {
        socklen_t length = sizeof(value);
        return GetOption(level, name, &value, length);
    }

    template <typename T>
    bool SetOption(int level, int name, const T& value)
    {
        socklen_t length = sizeof(value);
        return SetOption(level, name, &value, length);
    }

    // Get socket option with difference type
    template <typename Type, typename InternalType>
    bool GetOption(int level, int name, Type& value) const
    {
        InternalType internal_value;
        bool result = GetOption(level, name, internal_value);
        value = static_cast<Type>(internal_value);
        return result;
    }

    // Set socket option with difference type
    template <typename Type, typename InternalType>
    bool SetOption(int level, int name, const Type& value)
    {
        return SetOption(level, name, static_cast<InternalType>(value));
    }

    bool GetOption(int level, int name, bool& value) const
    {
        int int_value;
        bool result = GetOption(level, name, int_value);
        value = int_value != 0;
        return result;
    }

    bool SetOption(int level, int name, const bool& value)
    {
        return SetOption(level, name, static_cast<int>(value));
    }

    bool GetError(int& error)
    {
        return GetOption(SOL_SOCKET, SO_ERROR, error);
    }

    bool GetType(int& type) const
    {
        return GetOption(SOL_SOCKET, SO_TYPE, type);
    }

    bool GetSendBufferSize(size_t& size) const
    {
        return GetOption<size_t, int>(SOL_SOCKET, SO_SNDBUF, size);
    }

    bool SetSendBufferSize(size_t size)
    {
        return SetOption<size_t, int>(SOL_SOCKET, SO_SNDBUF, size);
    }

    bool GetReceiveBufferSize(size_t& size) const
    {
        return GetOption<size_t, int>(SOL_SOCKET, SO_RCVBUF, size);
    }

    bool SetReceiveBufferSize(size_t size)
    {
        return SetOption<size_t, int>(SOL_SOCKET, SO_RCVBUF, size);
    }

#ifdef _WIN32
    bool SetSendTimeout(int seconds, int msec = 0)
    {
        int option = seconds * 1000 + msec;
        return SetOption(SOL_SOCKET, SO_SNDTIMEO, option);
    }
    bool SetReceiveTimeout(int seconds, int msec = 0)
    {
        int option = seconds * 1000 + msec;
        return SetOption(SOL_SOCKET, SO_RCVTIMEO, option);
    }
    bool SetSendTimeout(const timeval& tv)
    {
        return SetSendTimeout(tv.tv_sec, tv.tv_usec / 1000);
    }
    bool SetReceiveTimeout(const timeval& tv)
    {
        return SetReceiveTimeout(tv.tv_sec, tv.tv_usec / 1000);
    }
#else
    bool SetSendTimeout(const timeval& tv)
    {
        return SetOption(SOL_SOCKET, SO_SNDTIMEO, tv);
    }
    bool SetReceiveTimeout(const timeval& tv)
    {
        return SetOption(SOL_SOCKET, SO_RCVTIMEO, tv);
    }
    bool SetSendTimeout(int seconds, int msec = 0)
    {
        timeval tv = { seconds, msec * 1000 };
        return SetSendTimeout(tv);
    }
    bool SetReceiveTimeout(int seconds, int msec = 0)
    {
        timeval tv = { seconds, msec * 1000 };
        return SetReceiveTimeout(tv);
    }
#endif
    bool Ioctl(int cmd, int& value)
    {
        return ioctlsocket(Handle(), cmd, reinterpret_cast<u_long*>(&value)) == 0;
    }

    bool SetBlocking(bool value = true)
    {
        return SocketSetNonblocking(Handle(), !value) == 0;
    }

    bool GetBlocking(bool &value)
    {
        int n = SocketGetNonblocking(Handle(), value);
        value = !value;
        return n == 0;
    }
    bool Bind(const SocketAddress& address)
    {
        return CheckError(bind(Handle(), address.Address(), address.Length()), "Bind");
    }

    bool GetLocalAddress(SocketAddress& address) const
    {
        socklen_t length = address.Capacity();
        if (CheckError(getsockname(m_handle, address.Address(), &length), "GetLocalAddress"))
        {
            address.SetLength(length);
            return true;
        }
        return false;
    }

    bool GetPeerAddress(SocketAddress& address) const
    {
        socklen_t length = address.Capacity();
        if (CheckError(getpeername(m_handle, address.Address(), &length), "GetPeerAddress"))
        {
            address.SetLength(length);
            return true;
        }
        return false;
    }

    bool GetReuseAddress(bool &value)
    {
        return GetOption(SOL_SOCKET, SO_REUSEADDR, value);
    }

    bool SetReuseAddress(bool value = true)
    {
        return SetOption(SOL_SOCKET, SO_REUSEADDR, value);
    }

    bool SetLinger(bool onoff = true, int timeout = 0)
    {
        struct linger l;
        l.l_onoff = onoff;
        l.l_linger = (u_short) timeout;
        return SetOption(SOL_SOCKET, SO_LINGER, l);
    }

    bool SetKeepAlive(bool onoff = true)
    {
        return SetOption(SOL_SOCKET, SO_KEEPALIVE, onoff);
    }

    bool GetKeepAlive(bool& onoff)
    {
        return GetOption(SOL_SOCKET, SO_KEEPALIVE, onoff);
    }

#if __unix__
    bool SetTcpKeepAliveOption(int idle, int interval, int count)
    {
        return
            SetOption(SOL_SOCKET, TCP_KEEPIDLE, idle) &&
            SetOption(SOL_SOCKET, TCP_KEEPINTVL, interval) &&
            SetOption(SOL_SOCKET, TCP_KEEPCNT, count);
    }
#endif
    bool SetTcpNoDelay(bool onoff = true)
    {
        return SetOption(IPPROTO_TCP, TCP_NODELAY, onoff);
    }

    bool GetTcpNoDelay(bool& onoff)
    {
        return GetOption(IPPROTO_TCP, TCP_NODELAY, onoff);
    }

    bool WaitReadable(struct timeval* tv = NULL, bool restart = true)
    {
        for (;;)
        {
            int n = Socket_PollReadable(Handle(), tv);
            if (n != SOCKET_ERROR_RETURN)
            {
                return n > 0;
            }
            else if (!IsInterruptedAndRestart(restart))
            {
                CheckError(n, "WaitReadable");
                break;
            }
        }
        return false;
    }

    bool WaitReadable(struct timeval& tv, bool restart = true)
    {
        return WaitReadable(&tv, restart);
    }

    bool WaitWriteable(struct timeval* tv = NULL, bool restart = true)
    {
        for (;;)
        {
            int n = Socket_PollWriteable(Handle(), tv);
            if (n != SOCKET_ERROR_RETURN)
            {
                return n > 0;
            }
            else if (!IsInterruptedAndRestart(restart))
            {
                CheckError(n, "WaitWriteable");
                break;
            }
        }
        return false;
    }

    bool WaitWriteable(struct timeval& tv, bool restart = true)
    {
        return WaitWriteable(&tv, restart);
    }

    bool IsReadable()
    {
        struct timeval tv = {0, 0};
        return WaitReadable(&tv);
    }

    bool IsWriteable()
    {
        struct timeval tv = {0, 0};
        return WaitWriteable(&tv);
    }
public:
    static int GetLastError()
    {
        return SocketGetLastError();
    }
    static std::string GetErrorString(int error)
    {
        return SocketGetErrorString(error);
    }
    static std::string GetLastErrorString()
    {
        return SocketGetErrorString(GetLastError());
    }
protected:
    void ReportError(const char* info) const
    {
        if (m_ExceptionEnabled)
            throw SocketError(info);
    }
    bool CheckError(int result, const char* info = "socket") const
    {
        if (result != 0)
        {
            ReportError(info);
            return false;
        }
        return true;
    }
    static void SetLastError(int error)
    {
        SocketSetLastError(error);
    }
    static void VerifyHandle(int fd)
    {
        assert(fd != InvalidHandle);
    }
    static bool IsInterruptedAndRestart(bool restart)
    {
        return restart && GetLastError() == SOCKET_ERROR_CODE(EINTR);
    }
private:
    SOCKET m_handle;
    bool m_ExceptionEnabled;
};

class ListenerSocket : public Socket
{
public:
    ListenerSocket()
    {
    }

    ListenerSocket(int af, int type, int protocol):
        Socket(socket(af, type, protocol))
    {
    }

    ListenerSocket(const SocketAddress& address, int type = SOCK_STREAM):
        Socket(socket(address.Family(), type, 0))
    {
        if (!IsValid())
            throw SocketError("ListenerSocket");
        if (!Bind(address))
        {
            throw std::runtime_error("Can't bind to " + address.ToString());
        }
    }

    using Socket::Create;

    bool Listen(int backlog = SOMAXCONN)
    {
        return CheckError(listen(Handle(), backlog), "Listen");
    }

    bool Accept(Socket& socket, bool auto_restart = true)
    {
        for (;;)
        {
            SOCKET s = accept(Handle(), NULL, NULL);
            if (s != InvalidHandle)
            {
                socket.Attach(s);
                return true;
            }
            else
            {
                if (!auto_restart || GetLastError() != SOCKET_ERROR_CODE(EINTR))
                {
                    ReportError("Accept");
                    break;
                }
            }
        }
        return false;
    }

    bool Accept(Socket& socket, SocketAddress& address, bool auto_restart = true)
    {
        socklen_t length = sizeof(address);
        for (;;)
        {
            SOCKET s = accept(Handle(), address.Address(), &length);
            if (s != InvalidHandle)
            {
                socket.Attach(s);
                address.SetLength(length);
                return true;
            }
            else
            {
                if (!auto_restart || GetLastError() != SOCKET_ERROR_CODE(EINTR))
                {
                    ReportError("Accept");
                    break;
                }
            }
        }
        return false;
    }
};

class DataSocket : public Socket
{
protected:
    DataSocket(){}
public:
    bool Connect(const SocketAddress& address)
    {
        if (connect(Handle(), address.Address(), address.Length()) != 0)
        {
            switch (errno)
            {
            case SOCKET_ERROR_CODE(EINTR):
            case SOCKET_ERROR_CODE(EWOULDBLOCK):
                return true;
            case SOCKET_ERROR_CODE(EINPROGRESS):
                {
                    bool blocking = true;
                    if (GetBlocking(blocking) && !blocking)
                        return true;
                }
            }
            ReportError("Connect");
            return false;
        }
        return true;
    }

    bool Send(
        const void* buffer,
        size_t buffer_size,
        size_t& sent_length,
        int flags = 0,
        bool auto_restart = true
        )
    {
        for (;;)
        {
            int n = send(Handle(), (const char*)buffer, buffer_size, flags);
            if (n != SOCKET_ERROR_RETURN)
            {
                sent_length = n;
                return true;
            }
            else
            {
                if (!IsInterruptedAndRestart(auto_restart))
                {
                    sent_length = 0;
                    ReportError("Send");
                    return false;
                }
            }
        }
    }

    bool Receive(
        void* buffer,
        size_t buffer_size,
        size_t& received_size,
        int flags = 0,
        bool auto_restart = true
        )
    {
        for (;;)
        {
            int n = recv(Handle(), (char*)buffer, buffer_size, flags);
            if (n != SOCKET_ERROR_RETURN)
            {
                received_size = n;
                return true;
            }
            else if (!IsInterruptedAndRestart(auto_restart))
            {
                received_size = 0;
                ReportError("Receive");
                return false;
            }
        }
    }

    /// receive in timeout
    /// @return false if error or timeout
    bool Receive(
        void* buffer,
        size_t buffer_size,
        size_t& received_size,
        timeval& timeout,
        int flags = 0,
        bool auto_restart = true
        )
    {
        if (WaitReadable(timeout, auto_restart))
            return Receive(buffer, buffer_size, received_size, flags, auto_restart);
        else
            return false;
    }
};

class StreamSocket : public DataSocket
{
public:
    StreamSocket(){}
    explicit StreamSocket(int af, int protocol)
    {
        if (!DataSocket::Create(af, SOCK_STREAM, protocol))
            throw SocketError("StreamSocket");
    }

    /// Create a stream socket
    bool Create(sa_family_t af = AF_INET, int protocol = 0)
    {
        return Socket::Create(af, SOCK_STREAM, protocol);
    }

    /// shutdown connection
    bool Shutdown()
    {
        return CheckError(SocketShutdown(Handle()), "Shutdown");
    }

    /// shutdown connection sending
    bool ShutdownSend()
    {
        return CheckError(SocketShutdownSend(Handle()), "ShutdownSend");
    }

    /// shutdown connection receiving
    bool ShutdownReceive()
    {
        return CheckError(SocketShutdownReceive(Handle()), "ShutdownReceive");
    }

    /// @brief Receive all length
    /// @return Whether received all data
    bool ReceiveAll(
        void *buffer,
        size_t buffer_size,
        size_t& received_size,
        int flags = 0,
        bool auto_restart = true
        )
    {
        assert((flags & MSG_PEEK) == 0);
#ifdef MSG_WAITALL
        flags |= MSG_WAITALL;
#endif
        received_size = 0;
        while (buffer_size > 0)
        {
            size_t n;
            if (Receive(buffer, buffer_size, n, flags, auto_restart))
            {
                if (n == 0)
                {
                    SetLastError(SOCKET_ERROR_CODE(ECONNRESET));
                    return false;
                }
                buffer = (char *) buffer + n;
                buffer_size -= n;
                received_size += n;
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    bool ReceiveAll(
        void *buffer,
        size_t buffer_size,
        int flags = 0,
        bool auto_restart = true
        )
    {
        size_t received_size;
        return ReceiveAll(buffer, buffer_size, received_size, flags, auto_restart);
    }

    /// @brief Receive all length
    /// @return Whether received all data
    bool ReceiveAll(
        void *buffer, size_t buffer_size, size_t& received_size,
        timeval& timeout, int flags = 0,
        bool auto_restart = true
        )
    {
        assert((flags & MSG_PEEK) == 0);
#ifdef MSG_WAITALL
        flags |= MSG_WAITALL;
#endif
        received_size = 0;
        while (buffer_size > 0)
        {
            if (WaitReadable(timeout, auto_restart))
            {
                size_t n;
                if (Receive(buffer, buffer_size, n, flags, auto_restart))
                {
                    if (n == 0)
                    {
                        SetLastError(SOCKET_ERROR_CODE(ECONNRESET));
                        return false;
                    }
                    buffer = (char *)buffer + n;
                    buffer_size -= n;
                    received_size += n;
                }
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    bool ReceiveAll(
        void *buffer,
        size_t buffer_size,
        timeval& timeout,
        int flags = 0,
        bool auto_restart = true
        )
    {
        size_t received_size;
        return ReceiveAll(buffer, buffer_size, received_size, timeout, flags, auto_restart);
    }

    /// Receive a line
    /// @return Whether received a complete line
    bool ReceiveLine(
        void* buffer,
        size_t buffer_size,
        size_t& received_size,
        size_t max_peek_size = 80
        )
    {
        received_size = 0;

        while (buffer_size > 0)
        {
            size_t peek_size = buffer_size > max_peek_size ? max_peek_size : buffer_size;
            size_t n;
            if (Receive(buffer, peek_size, n, MSG_PEEK) && n > 0)
            {
                char* p = (char*) memchr(buffer, '\n', n);
                if (p)
                {
                    bool result = ReceiveAll(buffer, p - (char*)buffer + 1, n);
                    received_size += n;
                    return result;
                }
                else
                {
                    bool result = ReceiveAll(buffer, n, n);
                    received_size += n;
                    if (!result)
                        return false;
                    buffer = (char*) buffer + n;
                    buffer_size -= n;
                }
            }
            else
            {
                return false;
            }
        }
        return false;
    }

    /// Receive a line
    /// @return Whether received a complete line
    bool ReceiveLine(std::string& str, size_t peek_size = 80)
    {
        const size_t kMaxPeekSize = 1024;
        char buffer[kMaxPeekSize];
        peek_size = peek_size > kMaxPeekSize ? kMaxPeekSize : peek_size;

        str.clear();

        for (;;)
        {
            size_t n;
            if (Receive(buffer, peek_size, n, MSG_PEEK) && n > 0)
            {
                char* p = (char*) memchr(buffer, '\n', n);
                if (p)
                {
                    bool result = ReceiveAll(buffer, p - buffer + 1, n);
                    str.append(buffer, n);
                    return result;
                }
                else
                {
                    bool result = ReceiveAll(buffer, n, n);
                    str.append(buffer, n);
                    if (!result)
                        return false;
                }
            }
            else
            {
                return false;
            }
        }
        return false;
    }

    /// Send all data of buffer to socket
    /// @return Whether all data sent
    bool SendAll(
        const void* buffer,
        size_t buffer_size,
        size_t& sent_size,
        int flags = 0,
        bool auto_restart = true
        )
    {
        sent_size = 0;
        while (buffer_size > 0)
        {
            size_t current_sent_size;
            if (Send(buffer, buffer_size, current_sent_size, flags, auto_restart))
            {
                buffer = (const char*)buffer + current_sent_size;
                buffer_size -= current_sent_size;
                sent_size += current_sent_size;
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    bool SendAll(
        const void* buffer,
        size_t buffer_size,
        int flags = 0,
        bool auto_restart = true
        )
    {
        size_t sent_size;
        return SendAll(buffer, buffer_size, sent_size, flags, auto_restart);
    }

    bool SendAll(
        const void* buffer,
        size_t buffer_size,
        size_t& sent_size,
        timeval& tv,
        int flags = 0,
        bool auto_restart = true
    )
    {
        sent_size = 0;
        while (buffer_size > 0)
        {
            if (WaitWriteable(tv, auto_restart))
            {
                size_t current_sent_size;
                if (Send(buffer, buffer_size, current_sent_size, flags, auto_restart))
                {
                    buffer = (const char*)buffer + current_sent_size;
                    buffer_size -= current_sent_size;
                    sent_size += current_sent_size;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        return true;
    }
};

class DatagramSocket : public DataSocket
{
public:
    DatagramSocket(int af, int protocol = 0)
    {
        if (!Create(af, protocol))
            throw SocketError("DatagramSocket");
    }
    DatagramSocket()
    {
    }

    bool Create(int af = AF_INET, int protocol = 0)
    {
        return Socket::Create(af, SOCK_DGRAM, protocol);
    }

    bool ReceiveFrom(
        void* buffer,
        size_t buffer_size,
        size_t& received_size,
        SocketAddress& address,
        int flags = 0
        )
    {
        socklen_t address_length = address.Capacity();
        int result = recvfrom(
            Handle(),
            (char*)buffer, buffer_size, flags,
            address.Address(), &address_length
        );
        if (result >= 0)
        {
            received_size = result;
            address.SetLength(address_length);
            return true;
        }
        else
        {
            received_size = 0;
            ReportError("ReceiveFrom");
        }
        return false;
    }

    bool SendTo(
        const void* buffer,
        size_t buffer_size,
        const SocketAddress& address,
        size_t& sent_size
        )
    {
        int n = sendto(
            Handle(), (const char*)buffer, buffer_size, 0,
            address.Address(), address.Length()
        );
        if (n >= 0)
        {
            sent_size = n;
            return true;
        }
        else
        {
            sent_size = 0;
            ReportError("SendTo");
            return false;
        }
    }
};

#endif // SYSTEM_NET_SOCKET_HPP
