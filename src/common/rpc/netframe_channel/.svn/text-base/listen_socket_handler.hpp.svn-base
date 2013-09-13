/// @date   05/05/2010
/// @author jeremychen (chenzheng)

#ifndef COMMON_RPC_LISTEN_SOCKET_HANDLER_HPP
#define COMMON_RPC_LISTEN_SOCKET_HANDLER_HPP

#include <stdio.h>
#include "common/netframe/socket_handler.hpp"
#include "common/rpc/netframe_channel/session_manager.hpp"
#include "common/rpc/netframe_channel/server_socket_handler.hpp"

namespace Rpc
{

class RpcListenSocketHandler : public netframe::ListenSocketHandler
{
public:
    RpcListenSocketHandler(netframe::NetFrame& netframe,
            SessionManager* session_manager,
            Rpc::Channel::EventHandler* handler,
            int priority
            ) :
        netframe::ListenSocketHandler(netframe),
        m_session_manager(session_manager),
        m_event_handler(handler),
        m_priority(priority)
    {
    }

    virtual netframe::StreamSocketHandler* OnAccepted(netframe::SocketId id)
    {
        ServerSocketHandler* handler = new ServerSocketHandler(
                GetNetFrame(), m_session_manager, m_event_handler, m_priority);
        return handler;
    }

    virtual void OnClose(int error_code)
    {
    }

private:
    SessionManager* m_session_manager;
    Rpc::Channel::EventHandler* m_event_handler;
    int m_priority;
};

}

#endif
