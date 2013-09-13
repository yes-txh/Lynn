/// @date 04/29/2010
/// @author jeremychen (chenzheng)

#ifndef COMMON_RPC_SESSION_HPP
#define COMMON_RPC_SESSION_HPP

#include <queue>
#include "common/rpc/types.hpp"
#include "common/rpc/channel.hpp"
#include "common/netframe/netframe.hpp"

namespace Rpc
{

class NetframeChannel;
class SessionManager;

class Session
{
public:
    /// Session的状态码
    enum Session_Status
    {
        Status_Init = 1,
        Status_Connecting = 2,
        Status_Connected = 3,
        Status_LogIn = 4,
        Status_LogOut = 5,
        Status_Disconnected = 6,
    };

    /// Session的类型码
    enum Session_Type
    {
        Type_Server = 1,
        Type_Client = 2,
    };

public:
    Session(netframe::NetFrame& netframe, const std::string& local_endpoint,
        const std::string& remote_endpoint, Session_Type type,
        Rpc::Channel::EventHandler* handler,
        int priority
    ) :
        m_netframe(netframe),
        m_local_endpoint(local_endpoint),
        m_remote_endpoint(remote_endpoint),
        m_remote_type(type),
        m_status(Status_Init),
        m_last_connect_time(0),
        m_connect_timeout(10),
        m_connect_number(0),
        m_max_message_length(2048 * 1024),
        m_max_connect_number(3),
        m_priority(priority),
        m_event_handler(handler)
    {
    }

    ~Session() {}

    /// @brief 注册会话
    void Login();

    /// @brief 注销会话
    void Logout();

    /// @brief 连接远端Server
    bool ConnectRemoteServer();

    /// @brief 重连远端Server
    bool ReconnectRemoteServer();

    /// @brief 设置状态
    void SetStatus(Session_Status status)
    {
        m_status = status;
    }

    /// @brief 获取状态
    int GetStatus() const
    {
        return m_status;
    }

    /// @brief 获取session名称标识
    std::string GetName() const
    {
        return m_local_endpoint + "/" + m_remote_endpoint;
    }

    static std::string GetSessionName(const std::string local_endpoint,
            const std::string remote_endpoint)
    {
        return local_endpoint + "/" + remote_endpoint;
    }

    /// @brief 获取socket fd
    int64_t GetSocketID() const
    {
        return m_socket_endpoint.GetId();
    }

    /// @brief 设置sock end point
    void SetSocketID(netframe::NetFrame::StreamEndPoint& endpoint)
    {
        m_socket_endpoint = endpoint;
    }

    const std::string& GetLocalEndPoint() const
    {
        return m_local_endpoint;
    }

    const std::string& GetRemoteEndPoint() const
    {
        return m_remote_endpoint;
    }

    /// @brief 处理未发送队列
    void ProcesWaitingSendPackets();

    /// @brief 设置Socket上所传输的最大数据包的大小
    void SetMaxPacketLen(int max_packet_len = 2048 * 1024)
    {
        m_max_message_length = max_packet_len;
    }

    /// @brief 设置最大重连次数
    void SetMaxConnectNumber(int number = 3)
    {
        m_max_connect_number = number;
    }

    /// @brief 设置连接的超时时间
    /// @param timeout 超时时间:秒
    void SetConnectTimeout(time_t timeout = 10)
    {
        m_connect_timeout = timeout;
    }

    /// @brief 判断对端是否不可达
    bool RemoteUnreachable() const;

    /// @brief 缓存发送失败的包
    void ReserveFailedPacket(netframe::Packet* packet);

    /// @brief 缓存发送失败的包
    void ReserveFailedPacket(const void* buffer, size_t size);

    /// @brief 发送数据
    Status_t Send(const void* buffer, size_t size);

    /// @brief 关闭session连接
    void CloseSession()
    {
        m_netframe.CloseEndPoint(m_socket_endpoint);
        m_status = Status_Disconnected;
    }

private:
    /// @brief 获取登录的本地断点
    void GetLoginEndPoint(std::string* endpoint);

    /// @brief 保持会话
    void Maintain();

    /// @brief 检查连接是否超时
    bool CheckConnectTimeout() const;

    /// @brief 更新最近一次的连接时间点
    void UpdateConnectTime(time_t time)
    {
        m_last_connect_time = time;
    }

private:
    SimpleMutex m_mutex;            ///< 发送失败缓冲区锁
    netframe::NetFrame& m_netframe; ///< 网络框架
    std::string m_local_endpoint;   ///< 会话本端的EndPoint
    std::string m_remote_endpoint;  ///< 会话对端的EndPoint
    Session_Type m_remote_type;     ///< 会话对端的类型: server/client
    Session_Status m_status;        ///< 会话状态
    time_t m_last_connect_time;     ///< Session上次连接的时间点
    time_t m_connect_timeout;       ///< Session连接超时的时间
    unsigned int m_connect_number;  ///< Session重连次数

    size_t m_max_message_length;    ///< Socket上所传输的最大正文大小
    size_t m_max_connect_number;    ///< 重连次数上限次数
    int m_priority;
    std::queue<netframe::Packet*> m_sending_queue;   ///< 发送失败缓冲区
    netframe::NetFrame::StreamEndPoint m_socket_endpoint;
    Rpc::Channel::EventHandler* m_event_handler;

    friend class SessionManager;    /// 友元类SessionManager
};

} // end namespace Rpc

#endif
