/// @date 05/06/2010
/// @author jeremychen (chenzheng)

#ifndef COMMON_RPC_NETFRAME_CHANNEL_HPP
#define COMMON_RPC_NETFRAME_CHANNEL_HPP

#include <set>
#include <string>
#include "common/rpc/channel.hpp"
#include "common/netframe/netframe.hpp"
#include "common/rpc/netframe_channel/session_manager.hpp"

namespace Rpc
{

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

class NetframeChannel : public Rpc::Channel
{
public:
    NetframeChannel(
        unsigned int netframe_thread = 4,
        unsigned int connect_timeout = 10,
        unsigned int max_reconnect_num = 3,
        unsigned int max_packet_len = 16 * 1024 * 1024,
        int priority = 0
    );

    ~NetframeChannel() {}

    /////////////以下为继承实现Rpc::Channel的函数///////////////

    /// @brief 向对端端点EndPoint发起连接
    virtual Status_t Connect(const std::string& remote_endpoint);

    /// @brief 主动断开对端端点EndPoint的连接
    virtual Status_t Disconnect(const std::string& remote_endpoint);

    /// @brief 向对端端点EndPoint发送数据
    virtual Status_t Send(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,
        const void* buffer,
        size_t size
    );

    /// @brief 在本端端点EndPoint上监听
    virtual Status_t Listen(const std::string& endpoint);

    /// @brief 设置EventHandler，返回旧的EventHandler
    virtual Channel::EventHandler* SetEventHandler(Channel::EventHandler* handler);

    /// @brief 获取本地EndPoint
    //
    virtual std::string GetLocalEndPoint();

    /// @brief 是否是有效的EndPoint
    virtual bool IsValidEndPoint(const std::string& endpoint);

    /// @brief 等待网络报文发送完毕，默认为10000毫秒
    virtual bool WaitForSendingComplete(int timeout = 10000)
    {
        return m_netframe.WaitForSendingComplete(timeout);
    }

    virtual void SetPriority(int priority)
    {
        m_priority = priority;
    }
private:
    /// @brief 是否是server端点
    static bool IsServerEndPoint(const std::string& endpoint);

    /// @brief 寻找session，不存在则创建
    Session* FindOrMakeSession(const std::string& local_endpoint,
            const std::string& remote_endpoint);

private:
    SimpleMutex m_mutex;
    netframe::NetFrame m_netframe;               ///< 实际的网络框架
    SessionManager m_session_manager;            ///< 会话管理器
    std::set<std::string> m_listen_endpoints;    ///< 监听的端点列表
    Rpc::Channel::EventHandler* m_event_handler; ///< 事件处理器指针

    unsigned int m_connect_timeout;              ///< 超时设置，秒
    unsigned int m_max_connect_number;           ///< 最大重连次数
    unsigned int m_max_packet_length;            ///< 最大包长
    int m_priority;
};

}

#endif
