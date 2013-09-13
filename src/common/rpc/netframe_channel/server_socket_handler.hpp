/// @date   05/05/2010
/// @author jeremychen (chenzheng)

#ifndef COMMON_RPC_SERVER_SOCKET_HANDLER_HPP
#define COMMON_RPC_SERVER_SOCKET_HANDLER_HPP

#include <stdio.h>
#include <iostream>
#include <utility>

#include "common/rpc/types.hpp"
#include "common/rpc/message.hpp"
#include "common/rpc/channel.hpp"
#include "common/netframe/packet.hpp"
#include "common/netframe/socket_handler.hpp"
#include "common/rpc/netframe_channel/session.hpp"
#include "common/rpc/netframe_channel/session_manager.hpp"
#include "common/rpc/netframe_channel/netframe_channel.hpp"

namespace Rpc
{

class ServerSocketHandler : public netframe::StreamSocketHandler
{
public:
    ServerSocketHandler(netframe::NetFrame& netframe,
            SessionManager* session_manager,
            Channel::EventHandler* handler,
            int priority) :
        netframe::StreamSocketHandler(netframe),
        m_session_manager(session_manager),
        m_event_handler(handler),
        m_priority(0)
    {
    }

    /// @brief  依据socket id查询会话
    /// @param  sockid socket id
    /// @return 返回socket id对应会话
    Session* FindSessionBySocketId(int64_t socket_id) const
    {
        return m_session_manager->FindBySocketId(socket_id);
    }

    void OnConnected()
    {
    }

    virtual void OnClose(int error_code)
    {
        Session* session = FindSessionBySocketId(GetEndPoint().GetId());
        if (NULL == session)
        {
            return;
        }

        session->SetStatus(Session::Status_Disconnected);
        GetNetFrame().CloseEndPoint(GetEndPoint());

        // 注册定时器
        m_session_manager->AddSessionCloseTimer(session);

        // 调用EventHandler相对应的接口
        const std::string& local_endpoint = session->GetLocalEndPoint();
        const std::string& remote_endpoint = session->GetRemoteEndPoint();
        m_event_handler->OnClosed(local_endpoint, remote_endpoint);
    }

    /// @brief 注册会话
    /// @param header 注册消息包
    void RegisterSession(const std::string& local_endpoint,
            const MessageHeader* header)
    {
        const char* remote_endpoint =
            static_cast<const LogInMessageHeader*>(header)->Identifier;
        bool session_exists = true;
        Session* session = m_session_manager->FindByEndPoint(local_endpoint,
                remote_endpoint);
        if (session == NULL)
        {
            session_exists = false;
            session = new Session(GetNetFrame(), local_endpoint,
                remote_endpoint, Session::Type_Client, m_event_handler, m_priority);
        }
        session->SetStatus(Session::Status_LogIn);
        session->SetSocketID(GetEndPoint());
        /// Add to Session Manager if session not exists.
        if (!session_exists)
        {
            m_session_manager->AddSession(session);
        }
    }

    /// @brief 返回注册成功ACK
    void SendLoginAck()
    {
        LogInAckMessageHeader ack(sizeof(LogInAckMessageHeader),
                sizeof(LogInAckMessageHeader), Rpc::Status_Success);
        if (GetNetFrame().SendPacket(GetEndPoint(), &ack, ack.Length) < 0)
        {
            // TODO
        }
    }

    virtual void OnReceived(const netframe::Packet& packet)
    {
        const MessageHeader* header = static_cast<const MessageHeader*>(
                (void*)packet.Content());

        switch (header->Type)
        {
        case MessageType_LogIn:
        {
            RegisterSession(packet.GetLocalAddress().ToString(), header);
            SendLoginAck();
            break;
        }
        case MessageType_LogOut:
        {
            std::string local_endpoint = packet.GetLocalAddress().ToString();
            std::string remote_endpoint =
                static_cast<const LogOutMessageHeader*>(header)->Identifier;
            Session* session = m_session_manager->FindByEndPoint(
                    local_endpoint, remote_endpoint);
            if (session)
            {
                // 设置logout，并不删除session
                session->SetStatus(Session::Status_LogOut);
            }
            break;
        }
        case MessageType_Invoke:
        case MessageType_Return:
        {
            size_t size = packet.Length();
            Session* session = FindSessionBySocketId(GetEndPoint().GetId());
            // 调用EventHandler相对应的OnReceived接口
            if (session)
            {
                m_event_handler->OnReceived(session->GetLocalEndPoint(),
                    session->GetRemoteEndPoint(), header, size);
            }
            break;
        }
        default:
            break;
        }
    }

    virtual bool OnSendingFailed(netframe::Packet* packet, int error_code)
    {
        Session* session = FindSessionBySocketId(GetEndPoint().GetId());
        if (!session)
            return true;

        // 对端不可达
        if (session->RemoteUnreachable())
        {
            m_event_handler->OnSendFailed(
                session->GetLocalEndPoint(),
                session->GetRemoteEndPoint(),
                packet->Content(),
                packet->Length()
            );
            return true;
        }
        else
        {
            session->ReserveFailedPacket(packet);
            return false;
        }
    }

    virtual int DetectPacketSize(const void* data, size_t length)
    {
        // 接收到包的长度小于RPC包头长度，继续接收
        if (length < sizeof(MessageHeader))
        {
            return 0;
        }
        const MessageHeader* header = static_cast<const MessageHeader*>(data);
        if (memcmp(header->Signature, "RPC", 4) != 0) // check the signature
        {
            return -1;
        }
        return static_cast<int>(header->Length);
    }

private:
    SessionManager* m_session_manager;              ///< 会话管理器
    Rpc::Channel::EventHandler* m_event_handler;    ///< 事件处理器
    int m_priority;
};

}

#endif
