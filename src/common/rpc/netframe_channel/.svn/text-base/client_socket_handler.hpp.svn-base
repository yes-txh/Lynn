/// @date   05/05/2010
/// @author jeremychen (chenzheng)

#ifndef COMMON_RPC_CLIENT_SOCKET_HANDLER_HPP
#define COMMON_RPC_CLIENT_SOCKET_HANDLER_HPP

#include <iostream>
#include <utility>

#include "common/rpc/types.hpp"
#include "common/rpc/channel.hpp"
#include "common/netframe/packet.hpp"
#include "common/netframe/socket_handler.hpp"
#include "common/rpc/netframe_channel/session.hpp"

namespace Rpc
{

class ClientSocketHandler : public netframe::StreamSocketHandler
{
public:
    ClientSocketHandler(netframe::NetFrame& netframe,
            Session* session,
            Rpc::Channel::EventHandler* handler) :
        netframe::StreamSocketHandler(netframe),
        m_session(session),
        m_event_handler(handler)
    {
    }

    virtual void OnConnected()
    {
        // 设置Session的状态为Connected
        m_session->SetStatus(Session::Status_Connected);
        // 设置Session的EndPoint
        m_session->SetSocketID(GetEndPoint());
        // 注册客户端的EndPoint
        m_session->Login();
    }

    virtual void OnClose(int error_code)
    {
        if (GetEndPoint().IsValid()) // 对端关闭时，主动关闭时已经关闭端点
        {
            GetNetFrame().CloseEndPoint(GetEndPoint());
        }
        m_session->SetStatus(Session::Status_Disconnected);
        // 回调Channel的OnClosed回调函数
        m_event_handler->OnClosed(m_session->GetLocalEndPoint(),
                m_session->GetRemoteEndPoint());
    }

    virtual void OnReceived(const netframe::Packet& packet)
    {
        const MessageHeader* header = static_cast<const MessageHeader*>
            ((void*)packet.Content());

        switch (header->Type)
        {
        // 判断消息类型为否为MessageType_LogInAck
        case MessageType_LogInAck:
        {
            // 依据回应包，设置Session状态为LogIn，立即发送缓存的包
            m_session->SetStatus(Session::Status_LogIn);
            // 回调Channel的OnConnected回调函数
            m_event_handler->OnConnected(m_session->GetLocalEndPoint(),
                    m_session->GetRemoteEndPoint());
            m_session->ProcesWaitingSendPackets();
            break;
        }
        case MessageType_Invoke:
        case MessageType_Return:
        {
            // 获取包的内容，长度
            const void* buffer = packet.Content();
            int size = packet.Length();

            // 回调Channel的OnReceived回调函数
            m_event_handler->OnReceived(
                m_session->GetLocalEndPoint(),
                m_session->GetRemoteEndPoint(),
                buffer,
                size
            );
            break;
        }
        default:
            break;
        }
    }

    virtual bool OnSendingFailed(netframe::Packet* packet, int error_code)
    {
        // 对端不可达
        if (m_session->RemoteUnreachable())
        {
            void* buffer = packet->Content();
            int size = packet->Length();

            //无法恢复的包调用该函数(调用多次后仍对端不可达)
            m_event_handler->OnSendFailed(
                m_session->GetLocalEndPoint(),
                m_session->GetRemoteEndPoint(),
                buffer,
                size
            );
            return true;
        }
        else
        {
            // 保存发送失败的包
            m_session->ReserveFailedPacket(packet);
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
    Session* m_session;    /// 会话指针
    Rpc::Channel::EventHandler* m_event_handler;
};

}

#endif
