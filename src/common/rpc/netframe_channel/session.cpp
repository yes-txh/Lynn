#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <vector>

#include "common/rpc/message.hpp"
#include "common/netframe/netframe.hpp"
#include "common/base/string/string_number.hpp"
#include "common/system/process.hpp"
#include "common/system/net/get_ip.hpp"
#include "common/system/net/socket.hpp"
#include "common/rpc/netframe_channel/session.hpp"
#include "common/rpc/netframe_channel/netframe_channel.hpp"
#include "common/rpc/netframe_channel/client_socket_handler.hpp"

namespace Rpc
{

struct LogInMessage : public LogInMessageHeader
{
    // Reserve space for identifier: host name + ip + pid
    char hostname[HOST_NAME_MAX + 24];
    LogInMessage(const char* identifier):
        LogInMessageHeader(0, 0)
    {
        size_t length = strlen(identifier);
        memcpy(Identifier, identifier, length + 1);
        Length = sizeof(LogInMessageHeader) + length + 1;
        HeaderLength = Length;
    }
};

struct LogOutMessage : public LogOutMessageHeader
{
    // Reserve space for identifier: host name + ip + pid
    char hostname[HOST_NAME_MAX + 24];
    LogOutMessage(const char* identifier):
        LogOutMessageHeader(0, 0)
    {
        size_t length = strlen(identifier);
        memcpy(Identifier, identifier, length + 1);
        Length = sizeof(LogOutMessageHeader) + length + 1;
        HeaderLength = Length;
    }
};

/// @brief ��ȡע���EndPoint
/// @param endpoint ����������ȡEndPoint
void Session::GetLoginEndPoint(std::string* endpoint)
{
    char hostname[HOST_NAME_MAX] = {0};
    gethostname(hostname, HOST_NAME_MAX);
    int pid = getpid();

    std::vector<IPAddress> iplist;
    GetLocalIpList(&iplist);

    endpoint->clear();
    if (iplist.size() > 0)
    {
        *endpoint = iplist[0].ToString() + ":";
    }
    *endpoint += std::string(hostname) + ":" + IntegerToString(pid);
}

/// @brief �Ự��½���÷������˵���ͻ��˵�EndPoint����ֹ����ʱ�˿ڲ�һ�����������
void Session::Login()
{
    // ���Ϳͻ��˵�½��MessageType_LogIn����Ϣ��
    std::string login_endpoint;
    GetLoginEndPoint(&login_endpoint);

    LogInMessage message(login_endpoint.c_str());
    if (m_netframe.SendPacket(m_socket_endpoint, &message, message.Length) < 0)
    {
        // TODO handle error
    }
}

/// @brief �Ựע�����÷�������ע���ͻ��˵�EndPoint
void Session::Logout()
{
    // ���Ϳͻ���ע����MessageType_LogOut����Ϣ��
    std::string logout_endpoint;
    GetLoginEndPoint(&logout_endpoint);

    LogOutMessage message(logout_endpoint.c_str());
    if (m_netframe.SendPacket(m_socket_endpoint, &message, message.Length) < 0)
    {
        // TODO handle error
    }
}

/// @brief �Ự���ֳ�����״̬
void Session::Maintain()
{
    switch (m_status)
    {
    case Status_Init:
    {
        ConnectRemoteServer();
        break;
    }
    case Status_Connecting:
    {
        if (CheckConnectTimeout())
        {
            ReconnectRemoteServer();
        }
        break;
    }
    case Status_Connected:
    {
        // already connected, reset connect number.
        m_connect_number = 0;
        break;
    }
    case Status_LogIn:
    case Status_LogOut:
        break;
    case Status_Disconnected:
    {
        if (m_remote_type == Type_Server)
        {
            ConnectRemoteServer();
        }
        break;
    }
    default:
        break;
    }
}

/// @brief �Ự����Զ�˷�����
bool Session::ConnectRemoteServer()
{
    SocketAddressStorage address;
    if (!address.Parse(m_remote_endpoint.c_str()))
    {
        return false;
    }

    netframe::StreamSocketHandler* pSocketHandler =
        new ClientSocketHandler(m_netframe, this, m_event_handler);

    size_t max_header_length = std::max(sizeof(InvokeMessageHeader), sizeof(ReturnMessageHeader));

    int64_t n = m_netframe.AsyncConnect(
                address,
                pSocketHandler,
                m_max_message_length + max_header_length,
                netframe::NetFrame::EndPointOptions()
                    .MaxCommandQueueLength(1024).Priority(m_priority)
            );

    if (n < 0)
    {
        delete pSocketHandler;
        return false;
    }
    else
    {
        SetStatus(Status_Connecting);
        UpdateConnectTime(time(NULL));
        m_connect_number++;
    }

    return true;
}

/// @brief ����Զ�˷�����
bool Session::ReconnectRemoteServer()
{
    if (m_socket_endpoint.IsValid())
    {
        m_netframe.CloseEndPoint(m_socket_endpoint);
    }
    return ConnectRemoteServer();
}

/// @brief  ��������Ƿ�ʱ
/// @return true:  ��ʱ
/// @return false: δ��ʱ
bool Session::CheckConnectTimeout() const
{
    if (difftime(time(NULL), m_last_connect_time) > m_connect_timeout)
    {
        return true;
    }
    return false;
}

/// @brief  ��������
/// @param  buffer �������ݻ�����
/// @param  size �������ݻ������ĳ���
/// @return �ɹ�����ʧ��
Status_t Session::Send(const void* buffer, size_t size)
{
    // ���ͻ��������İ�
    ProcesWaitingSendPackets();

    // ���͵�ǰ����
    netframe::Packet* packet = new netframe::Packet();
    packet->SetContent(buffer, size);
    if (m_netframe.SendPacket(m_socket_endpoint, packet) < 0)
    {
        ReserveFailedPacket(packet);
    }

    return Rpc::Status_Success;
}

/// @brief ������ʧ�ܵİ������ȷ���
void Session::ProcesWaitingSendPackets()
{
    MutexLocker locker(m_mutex);
    while (!m_sending_queue.empty())
    {
        netframe::Packet* packet = m_sending_queue.front();
        if (m_netframe.SendPacket(m_socket_endpoint, packet) == 0)
        {
            m_sending_queue.pop();
        }
        else
        {
            break;
        }
    }
}

/// @brief ���淢��ʧ�ܵİ�
/// @param packet ����ʧ�ܵİ�
void Session::ReserveFailedPacket(netframe::Packet* packet)
{
    MutexLocker locker(m_mutex);
    m_sending_queue.push(packet);
}

/// @brief  ���淢��ʧ�ܵİ�
/// @param  buffer ����������ַ
/// @param  size ���������ĳ���
void Session::ReserveFailedPacket(const void* buffer, size_t size)
{
    MutexLocker locker(m_mutex);
    netframe::Packet* packet = new netframe::Packet;
    packet->SetContent(buffer, size);
    m_sending_queue.push(packet);
}

/// @brief  �ж϶Զ��Ƿ�ɴ�
/// @return true:  ���ɴ�
/// @return false: �ɴ�
bool Session::RemoteUnreachable() const
{
    if (m_connect_number > m_max_connect_number)
    {
        return true;
    }
    return false;
}

} // end namespace Rpc
