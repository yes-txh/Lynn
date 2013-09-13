#include <stdio.h>
#include <vector>

#include "common/rpc/types.hpp"
#include "common/rpc/message.hpp"
#include "common/base/string/string_number.hpp"
#include "common/system/net/get_ip.hpp"
#include "common/system/net/socket.hpp"
#include "common/rpc/netframe_channel/session.hpp"
#include "common/rpc/netframe_channel/session_manager.hpp"
#include "common/rpc/netframe_channel/netframe_channel.hpp"
#include "common/rpc/netframe_channel/listen_socket_handler.hpp"

#ifdef _WIN32
#include <process.h>
#endif

namespace Rpc
{

NetframeChannel::NetframeChannel(
        unsigned int netframe_thread,
        unsigned int connect_timeout,
        unsigned int max_reconnect_num,
        unsigned int max_packet_len,
        int priority) :
        m_netframe(netframe_thread),
        m_connect_timeout(connect_timeout),
        m_max_connect_number(max_reconnect_num),
        m_max_packet_length(max_packet_len),
        m_priority(0)
{
    m_event_handler = NULL;
}

/// @brief  连接到指定的EndPoint
/// @param  endpoint 服务器端的EndPoint
/// @return 成功或者失败
Status_t NetframeChannel::Connect(const std::string& remote_endpoint)
{
    SocketAddressStorage address;
    if (!address.Parse(remote_endpoint.c_str()))
    {
        return Rpc::Status_InvalidEndPoint;
    }

    // 创建Session
    Session* session = m_session_manager.FindByEndPoint(
            GetLocalEndPoint(), remote_endpoint);
    bool session_exists = true;
    if (session == NULL)
    {
        session = new Session(m_netframe, GetLocalEndPoint(), remote_endpoint,
                              Session::Type_Server, m_event_handler, m_priority);
        session_exists = false;
    }

    session->SetConnectTimeout(m_connect_timeout);
    session->SetMaxConnectNumber(m_max_connect_number);
    session->SetMaxPacketLen(m_max_packet_length);

    // 会话创建后，立即发起连接
    session->SetStatus(Session::Status_Connecting);

    if (!session->ConnectRemoteServer())
    {
        if (!session_exists) // 新创建的Session，需要清理
        {
            delete session;
        }
        return Rpc::Status_ConnectionError;
    }

    // 加入Session管理器
    if (!session_exists)
    {
        m_session_manager.AddSession(session);
    }

    return Rpc::Status_Success;
}

/// @brief  客户端调用，断开到服务器的连接，服务器不主动关闭连接
/// @param  endpoint 服务器端的EndPoint
/// @return 成功或者失败
Status_t NetframeChannel::Disconnect(const std::string& remote_endpoint)
{
    // 依据EndPoint查找会话
    Session* session = m_session_manager.FindByEndPoint(
            GetLocalEndPoint(), remote_endpoint);
    if (session)
    {
        session->Logout();
        session->CloseSession();
        // 延迟删除，注册定时器
        m_session_manager.AddSessionCloseTimer(session);
    }
    return Rpc::Status_Success;
}

std::string NetframeChannel::GetLocalEndPoint()
{
    char hostname[HOST_NAME_MAX] = {0};
    gethostname(hostname, HOST_NAME_MAX);
    int pid = getpid();

    std::string endpoint;
    std::vector<IPAddress> iplist;
    GetLocalIpList(&iplist);
    if (iplist.size() > 0)
    {
        endpoint = iplist[0].ToString() + ":";
    }
    endpoint += std::string(hostname) + ":" + IntegerToString(pid);
    return endpoint;
}

Session* NetframeChannel::FindOrMakeSession(
    const std::string& local_endpoint,
    const std::string& remote_endpoint
)
{
    Session* session = NULL;
    if (IsServerEndPoint(remote_endpoint))  // 远端为服务器
    {
        std::string local;

        if (local_endpoint.empty())
            local = GetLocalEndPoint();
        else
            local = local_endpoint;

        MutexLocker locker(m_mutex);
        session = m_session_manager.FindByEndPoint(local, remote_endpoint);

        if (!session)
        {
            // 连接失败
            if (Connect(remote_endpoint) == Rpc::Status_Success)
                session = m_session_manager.FindByEndPoint(local, remote_endpoint);
        }
    }
    else  // 远端为客户端
    {
        MutexLocker locker(m_mutex);
        for (std::set<std::string>::iterator i = m_listen_endpoints.begin();
                i != m_listen_endpoints.end(); ++i)
        {
            session = m_session_manager.FindByEndPoint(*i, remote_endpoint);
            if (session)
                break;
        }
    }

    return session;
}

/// @brief  向指定的EndPoint发送数据
/// @param  endpoint 发送数据到EndPoint
/// @param  buffer 发送数据缓冲区
/// @param  size 发送数据缓冲区的长度
/// @return 成功或者失败
Status_t NetframeChannel::Send(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,
    const void* buffer,
    size_t size
)
{
    // 第一次发送时，进行自动连接
    Session* session = FindOrMakeSession(local_endpoint, remote_endpoint);
    if (session != NULL)
    {
        if (session->GetStatus() == Session::Status_LogIn)
        {
            session->Send(buffer, size);
        }
        else
        {
            // 连接已建立，缓存发送包
            session->ReserveFailedPacket(buffer, size);
        }
        return Rpc::Status_Success;
    }
    // 连接未建立，返回连接错误
    return Rpc::Status_ConnectionError;
}

/// @brief  在指定的EndPoint上监听
/// @param  endpoint 监听的EndPoint
/// @return 成功或者失败
Status_t NetframeChannel::Listen(const std::string& endpoint)
{
    SocketAddressStorage address;
    if (!address.Parse(endpoint.c_str()))
    {
        return Rpc::Status_InvalidEndPoint;
    }

    RpcListenSocketHandler* pListenSocketHandler = new RpcListenSocketHandler(
            m_netframe, &m_session_manager, m_event_handler, m_priority);

    if (m_netframe.AsyncListen(address,
                pListenSocketHandler,
                m_max_packet_length,
                netframe::NetFrame::EndPointOptions()
                    .MaxCommandQueueLength(1024)
                    .Priority(m_priority)
                ) < 0)
    {
        delete pListenSocketHandler;
        if (errno == EADDRINUSE) // port already in use
            return Rpc::Status_EndPointInUse;
        return Rpc::Status_InvalidEndPoint;
    }
    m_listen_endpoints.insert(endpoint);

    return Rpc::Status_Success;
}

/// @brief  设置Packact EventHandler,返回旧的EventHandler
/// @param  handler 新的EventHandler
/// @return 旧的EventHandler
Rpc::Channel::EventHandler* NetframeChannel::SetEventHandler(
        Rpc::Channel::EventHandler* handler)
{
    Rpc::Channel::EventHandler* old = m_event_handler;
    m_event_handler = handler;
    return old;
}

bool NetframeChannel::IsServerEndPoint(const std::string& endpoint)
{
    SocketAddressInet address;
    return address.Parse(endpoint.c_str());
}

bool NetframeChannel::IsValidEndPoint(const std::string& endpoint)
{
    SocketAddressStorage address;
    return address.Parse(endpoint.c_str());
}

}
