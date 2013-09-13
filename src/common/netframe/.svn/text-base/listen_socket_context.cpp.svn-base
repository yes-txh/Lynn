#include "common/netframe/listen_socket_context.hpp"
#include "common/netframe/stream_socket_context.hpp"
#include "common/netframe/event_poller.hpp"
#include "common/netframe/command_event.hpp"
#include "common/netframe/netframe.hpp"
#include "common/system/concurrency/thread.hpp"
#include "common/system/net/socket.hpp"

namespace netframe {

unsigned int ListenSocketContext::GetWantedEventMask() const
{
    return IoEvent_Acceptable;
}

int ListenSocketContext::HandleIoEvent(const IoEvent& event)
{
    // 无论什么实际都是 acceptable 事件
    return HandleAccept();
}

int ListenSocketContext::HandleCommandEvent(const CommandEvent& event)
{
    switch (event.Type)
    {
    case CommandEvent_AddSocket: // 无需实际处理
        return 1;
    case CommandEvent_CloseSocket:
        return -1;
    default:
        break;
    }
    return 0;
}

// 在监听端口上接收新的连接请求事件
int ListenSocketContext::HandleAccept()
{
    SocketAddressStorage address;

    for (;;)
    {
        int new_fd = -1;

        // 接收连接
        socklen_t address_length = address.Capacity();
        do
        {
            new_fd = (int) ::accept(GetFd(), address.Address(), &address_length);
        }
        while ((new_fd < 0) && (ThisThread::GetLastErrorCode() == EINTR));

        if (new_fd >= 0)
        {
            StreamSocket socket;
            socket.Attach(new_fd);

            address.SetLength(address_length);
            ListenSocketHandler* listen_handler = GetEventHandler();
            SocketId id = NetFrame::GenerateSocketId(new_fd);
            StreamSocketHandler* new_handler = listen_handler->OnAccepted(id);

            if (new_handler != NULL)
            {
                // 设置非阻塞,在新Socket上请求读写事件
                Socket::SetCloexec(new_fd, true);
                socket.SetBlocking(false);
                socket.SetTcpNoDelay();
                socket.SetKeepAlive();
                socket.SetSendBufferSize(m_EndPointOptions.SendBufferSize());
                socket.SetReceiveBufferSize(m_EndPointOptions.ReceiveBufferSize());
                int priority = m_EndPointOptions.Priority();
                if (priority)
                {
#ifndef _WIN32
                    socket.SetOption(SOL_IP, IP_TOS, priority << 5);
                    socket.SetOption(SOL_SOCKET, SO_PRIORITY, priority);
#endif
                }

                new_handler->SetEndPointId(id);
                new_handler->OnConnected();
                new_handler->SetRemoteAddress(address);
                StreamSocketContext* new_socket_context = new StreamSocketContext(
                    m_NetFrame, &m_LocalAddress, &address, id, new_handler,
                    m_MaxPacketLength, m_EndPointOptions,
                    true // connected
                );

                socket.Detach();
                m_NetFrame->AddCommandEvent(
                    CommandEvent(CommandEvent_AddSocket, id,
                        static_cast<SocketContext*>(new_socket_context))
                );
            }
        }
        else
        {
            // TODO handle fatal and nonfatal errors here
            break;
        }
    }
    return 1;
}

} // namespace netframe

