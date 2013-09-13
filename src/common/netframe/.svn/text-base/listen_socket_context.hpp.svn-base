#ifndef COMMON_NETFRAME_LISTEN_SOCKET_CONTEXT_HPP
#define COMMON_NETFRAME_LISTEN_SOCKET_CONTEXT_HPP

#include "common/netframe/socket_context.hpp"

namespace netframe {

class NetFrame;

class ListenSocketContext : public SocketContext {
public:
    ListenSocketContext(
        NetFrame* netframe,
        const SocketAddress* local_address,
        SocketId sock_id,
        ListenSocketHandler* handler,
        size_t max_packet_size,
        const NetFrame::EndPointOptions& options
    ) : SocketContext(netframe, local_address, NULL, sock_id, max_packet_size, options)
    {
        SetEventHandler(handler);
    }
    virtual int HandleIoEvent(const IoEvent& event); // override
    virtual int HandleCommandEvent(const CommandEvent& event); // override

private:
    /// 在监听端口上接收新的连接请求事件
    int HandleAccept();
    virtual unsigned int GetWantedEventMask() const; // override

    virtual ListenSocketHandler* GetEventHandler() const
    {
        return static_cast<ListenSocketHandler*>(
            SocketContext::GetEventHandler());
    }
};

} // namespace netframe

#endif // COMMON_NETFRAME_LISTEN_SOCKET_CONTEXT_HPP
