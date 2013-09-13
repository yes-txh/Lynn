#include "common/netframe/netframe.hpp"
#include "common/netframe/command_event.hpp"
#include "common/netframe/work_thread.hpp"
#include "common/netframe/listen_socket_context.hpp"
#include "common/netframe/stream_socket_context.hpp"
#include "common/netframe/datagram_socket_context.hpp"
#include "common/system/net/socket.hpp"
#include "common/system/concurrency/thread.hpp"
#include "common/system/system_information.h"
#include "common/system/memory/mempool.hpp"
#include "common/system/concurrency/atomic/atomic.h"

namespace netframe {

using namespace std;

uint32_t NetFrame::m_SequenceId = 0;

NetFrame::NetFrame(unsigned int num_workthreads, size_t max_bufferd_size):
    m_CurrentBufferedMemorySize(0), m_BufferedPacketNumber(0)
{
#ifdef _WIN32
    WSADATA wsaData;
    ::WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif //_WIN32
    if (num_workthreads == 0)
    {
        num_workthreads = GetLogicalCpuNumber();
    }

    m_MaxBufferedMemorySize = max_bufferd_size;
    MemPool_Initialize(MAX_MEMUNIT_SIZE);

    for (size_t i = 0; i < num_workthreads; ++i)
    {
        WorkThread* work_thread = new WorkThread();
        m_WorkThreads.push_back(work_thread);
    }
    for (size_t i = 0; i < num_workthreads; ++i)
    {
        m_WorkThreads[i]->Start();
    }
}

NetFrame::~NetFrame()
{
    for (size_t i = 0; i < m_WorkThreads.size(); ++i)
    {
        delete m_WorkThreads[i];
    }
    m_WorkThreads.clear();

    // 用于诊断错误
    m_MaxBufferedMemorySize = 0;
    m_CurrentBufferedMemorySize = 0;
    m_BufferedPacketNumber = 0;
#ifdef _WIN32
    ::WSACleanup();
#endif //_WIN32
}

WorkThread* NetFrame::GetWorkThreadOfFd(int fd)
{
#ifdef _WIN32
    // socket on win32 is always multiple of 4
    size_t index = (fd / 4) % m_WorkThreads.size();
#else
    size_t index = fd % m_WorkThreads.size();
#endif
    return m_WorkThreads[index];
}

int64_t NetFrame::AsyncListen(
    const SocketAddress& address,
    ListenSocketHandler* handler,   // Socket上的处理器
    size_t max_packet_size,         // Socket上所传输的最大数据包的大小
    const EndPointOptions& options
)
{
    if (handler == NULL)
    {
        return -EINVAL;
    }

    ListenerSocket listener;
    if (!listener.Create(address.Family(), SOCK_STREAM))
    {
        return -ThisThread::GetLastErrorCode();
    }

    listener.SetCloexec();
    listener.SetBlocking(false);
    listener.SetReceiveBufferSize(options.ReceiveBufferSize());
    listener.SetSendBufferSize(options.SendBufferSize());
    listener.SetReuseAddress();

    if (!listener.Bind(address))
        return -ThisThread::GetLastErrorCode();

    if (!listener.Listen())
        return -ThisThread::GetLastErrorCode();

    SocketId sock_id = GenerateSocketId(listener.Handle());
    ListenSocketContext* socket_context = new ListenSocketContext(
        this,
        &address,
        sock_id,
        handler,
        max_packet_size,
        options
    );

    listener.Detach();
    handler->SetEndPointId(sock_id);

    if (!AddCommandEvent(CommandEvent(CommandEvent_AddSocket, sock_id,
                    static_cast<SocketContext*>(socket_context))))
    {
        delete socket_context;
        return -EINVAL;
    }

    return sock_id.Id;
}

int64_t NetFrame::AsyncConnect(
    const SocketAddress* local_address,
    const SocketAddress& remote_address,
    StreamSocketHandler* handler,   ///< Socket上的处理器
    size_t max_packet_size,         ///< Socket上所传输的最大数据包的大小
    const EndPointOptions& options
)
{
    if (handler == NULL)
    {
        return -EINVAL;
    }

    StreamSocket socket;
    if (!socket.Create(remote_address.Family()))
        return -ThisThread::GetLastErrorCode();

    socket.SetCloexec();
    socket.SetBlocking(false);

    if (local_address && !socket.Bind(*local_address))
        return -ThisThread::GetLastErrorCode();

    socket.SetTcpNoDelay();
    socket.SetKeepAlive();

    if (!socket.Connect(remote_address))
    {
        int error = ThisThread::GetLastErrorCode();
        if ((error != EAGAIN) && (error != EINPROGRESS))
            return -error;
    }

    // 设置发送缓冲区和接收缓冲区的大小
    socket.SetSendBufferSize(options.SendBufferSize());
    socket.SetReceiveBufferSize(options.ReceiveBufferSize());

    int priority = options.Priority();
    if (priority)
    {
		// win32下不需要设置,否则编译会有点问题
#ifndef _WIN32
        socket.SetOption(SOL_IP, IP_TOS, priority << 5);
        socket.SetOption(SOL_SOCKET, SO_PRIORITY, priority);
#endif
    }

    SocketId sock_id = GenerateSocketId(socket.Handle());
    SocketContext* socket_context = new StreamSocketContext(
        this, local_address, &remote_address, sock_id,
        handler, max_packet_size, options,
        false // not connected
    );

    socket.Detach();
    handler->SetEndPointId(sock_id);

    handler->SetRemoteAddress(remote_address);

    if (!AddCommandEvent(CommandEvent(CommandEvent_AddSocket, sock_id, socket_context)))
    {
        delete socket_context;
        return -EINVAL;
    }
    return sock_id.Id;
}

int64_t NetFrame::AsyncConnect(
    const SocketAddress& local_address,
    const SocketAddress& remote_address,
    StreamSocketHandler* handler,   ///< Socket上的处理器
    size_t  max_packet_size,        ///< Socket上所传输的最大数据包的大小
    const EndPointOptions&  options
)
{
    return AsyncConnect(&local_address, remote_address, handler, max_packet_size, options);
}

int64_t NetFrame::AsyncConnect(
    const SocketAddress& remote_address,
    StreamSocketHandler* handler,       ///< Socket上的处理器
    size_t max_packet_size,             ///< Socket上所传输的最大数据包的大小
    const EndPointOptions& options
)
{
    return AsyncConnect(NULL, remote_address, handler, max_packet_size, options);
}

int64_t NetFrame::AsyncDatagramBind(
    const SocketAddress& address,
    DatagramSocketHandler* handler, ///< Socket上的处理器
    size_t max_packet_size,         ///< Socket上所传输的最大数据包的大小
    const EndPointOptions& options
)
{
    if (handler == NULL)
    {
        return -EINVAL;
    }

    DatagramSocket socket;
    if (!socket.Create(address.Family()))
        return -ThisThread::GetLastErrorCode();

    socket.SetCloexec();
    socket.SetBlocking(false);
    socket.SetSendBufferSize(options.SendBufferSize());
    socket.SetReceiveBufferSize(options.ReceiveBufferSize());
    int priority = options.Priority();
    if (priority)
    {
#ifndef _WIN32
        socket.SetOption(SOL_IP, IP_TOS, priority << 5);
        socket.SetOption(SOL_SOCKET, SO_PRIORITY, priority);
#endif
    }

    if (!socket.Bind(address))
        return -ThisThread::GetLastErrorCode();

    SocketId sock_id = GenerateSocketId(socket.Handle());
    SocketContext* socket_context = new DatagramSocketContext(
        this, &address, NULL, sock_id, handler, max_packet_size, options);

    socket.Detach(); // 防止fd析构时被关闭
    handler->SetEndPointId(sock_id);

    if (!AddCommandEvent(CommandEvent(CommandEvent_AddSocket, sock_id, socket_context)))
    {
        delete socket_context;
        return -EINVAL;
    }

    return sock_id.Id;
}

bool NetFrame::AddCommandEvent(const CommandEvent& event)
{
    WorkThread* work_thread = GetWorkThreadOfFd(event.GetFd());
    if (work_thread != NULL)
    {
        work_thread->AddCommandEvent(event);
    }
    else
    {
        return false;
    }

    return true;
}

int NetFrame::CloseEndPoint(EndPoint& endpoint, bool immidiate)
{
    int fd = endpoint.GetFd();
    if (fd < 0)
        return -1;

    WorkThread* work_thread = GetWorkThreadOfFd(fd);
    if (work_thread)
    {
        if (immidiate) // 立即删除一个端口
        {
            work_thread->AddCommandEvent(CommandEvent(
                        CommandEvent_CloseSocket_Now, endpoint.GetSockId()));
        }
        else // 关闭一个端口，等待之前的数据包发完
        {
            work_thread->AddCommandEvent(CommandEvent(CommandEvent_CloseSocket,
                        endpoint.GetSockId()));
        }
        return 0;
    }
    return -1;
}

int NetFrame::SendPacket(const StreamEndPoint& endpoint, const void* data,
        size_t size, bool urgent)
{
    if (!endpoint.IsValid())
        return -1;

    Packet* packet = new Packet();
    packet->SetContent(data, size);
    int result = SendPacket(endpoint, packet, urgent);
    if (result < 0)
    {
        delete packet;
    }
    return result;
}

int NetFrame::SendPacket(const StreamEndPoint& endpoint, Packet* packet, bool urgent)
{
    if (!endpoint.IsValid())
        return -1;

    size_t total_buffered_length = GetCurrentBufferedLength();
    CommandEventType command_type = urgent ? CommandEvent_SendUrgentPacket :
                                             CommandEvent_SendPacket;
    // 内存配额没有用完或为紧急数据包
    if((total_buffered_length + packet->Length()) < m_MaxBufferedMemorySize || urgent)
    {
        WorkThread* work_thread = GetWorkThreadOfFd(endpoint.GetFd());
        if (work_thread)
        {
            IncreaseBufferedLength(packet->Length());
            IncreaseBufferedPacket();
            work_thread->AddCommandEvent(CommandEvent(command_type, endpoint.GetSockId(), packet));
            return 0;
        }
    }
    return -1;
}

int NetFrame::SendPacket(const DatagramEndPoint& endpoint,
        const SocketAddress& address, Packet* packet)
{
    if (!endpoint.IsValid())
        return -1;

    packet->SetRemoteAddress(address);
    WorkThread* work_thread = GetWorkThreadOfFd(endpoint.GetFd());
    if (work_thread)
    {
        IncreaseBufferedLength(packet->Length());
        IncreaseBufferedPacket();
        work_thread->AddCommandEvent(CommandEvent(CommandEvent_SendPacket,
                    endpoint.GetSockId(), packet));
        return 0;
    }
    return -1;
}

int NetFrame::SendPacket(
    const DatagramEndPoint& endpoint,
    const SocketAddress& address,
    const void* data, size_t size
)
{
    if (!endpoint.IsValid())
        return -1;

    Packet* packet = new Packet;
    packet->SetContent(data, size);
    int result = SendPacket(endpoint, address, packet);
    if (result < 0)
    {
        delete packet;
    }
    return result;
}

/************************************************************************/
/* 底层调用函数                                                         */
/************************************************************************/

int NetFrame::SetNonBlock(int fd)
{
#if _WIN32
    u_long one = 1;
    int err = ::ioctlsocket(fd, FIONBIO, &one);
#elif __unix__
    int flag = ::fcntl(fd, F_GETFL, 0);
    int err = ::fcntl(fd, F_SETFL, flag | O_NONBLOCK);
#endif //_WIN32
    return err;
}

int NetFrame::SetReceiveBufferSize(int fd, size_t size)
{
    int bufSize = (int)size;
    return ::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&bufSize, sizeof(bufSize));
}

int NetFrame::SetSendBufferSize(int fd, size_t size)
{
    int bufSize = (int)size;
    return ::setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&bufSize, sizeof(bufSize));
}

int NetFrame::SetKeepAlive(int fd)
{
    int one = 1;
    return ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&one, sizeof(one));
}

int NetFrame::SetTcpNoDelay(int fd)
{
#ifdef _WIN32
    int one = 1;
    return ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(one));
#else
    return 0;
#endif
}

int NetFrame::CloseFd(int fd)
{
#if _WIN32
    return closesocket(fd);
#elif __unix__
    return close(fd);
#endif
}

void NetFrame::IncreaseBufferedLength(size_t length)
{
    AtomicAdd(m_CurrentBufferedMemorySize, length);
}

void NetFrame::DecreaseBufferedLength(size_t length)
{
    AtomicSub(m_CurrentBufferedMemorySize, length);
}

size_t NetFrame::GetCurrentBufferedLength() const
{
    return AtomicGet(m_CurrentBufferedMemorySize);
}

size_t NetFrame::GetMaxBufferedLength() const
{
    return  m_MaxBufferedMemorySize;
}

void NetFrame::IncreaseBufferedPacket()
{
    AtomicIncrement(m_BufferedPacketNumber);
}

void NetFrame::DecreaseBufferedPacket()
{
    AtomicDecrement(m_BufferedPacketNumber);
}

size_t NetFrame::GetBufferedPacketNumber()
{
    return AtomicGet(m_BufferedPacketNumber);
}

bool NetFrame::WaitForSendingComplete(int timeout)
{
    int time_interval = 10;
    int total_wait_time = 0;
    while (GetBufferedPacketNumber() > 0)
    {
        ThisThread::Sleep(time_interval);
        total_wait_time += time_interval;
        if (total_wait_time >= timeout && timeout > 0)
            break;
    }
    if (GetBufferedPacketNumber() > 0)
        return false;
    return true;
}

/// 由sock fd和序列号拼接成64位的id
SocketId NetFrame::GenerateSocketId(int32_t sock_fd)
{
    SocketId id;
    id.SequenceId = AtomicIncrement(m_SequenceId);
    id.SockFd = sock_fd;
    return id;
}

} // namespace netframe

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
