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

    /////////////����Ϊ�̳�ʵ��Rpc::Channel�ĺ���///////////////

    /// @brief ��Զ˶˵�EndPoint��������
    virtual Status_t Connect(const std::string& remote_endpoint);

    /// @brief �����Ͽ��Զ˶˵�EndPoint������
    virtual Status_t Disconnect(const std::string& remote_endpoint);

    /// @brief ��Զ˶˵�EndPoint��������
    virtual Status_t Send(
        const std::string& local_endpoint,
        const std::string& remote_endpoint,
        const void* buffer,
        size_t size
    );

    /// @brief �ڱ��˶˵�EndPoint�ϼ���
    virtual Status_t Listen(const std::string& endpoint);

    /// @brief ����EventHandler�����ؾɵ�EventHandler
    virtual Channel::EventHandler* SetEventHandler(Channel::EventHandler* handler);

    /// @brief ��ȡ����EndPoint
    //
    virtual std::string GetLocalEndPoint();

    /// @brief �Ƿ�����Ч��EndPoint
    virtual bool IsValidEndPoint(const std::string& endpoint);

    /// @brief �ȴ����籨�ķ�����ϣ�Ĭ��Ϊ10000����
    virtual bool WaitForSendingComplete(int timeout = 10000)
    {
        return m_netframe.WaitForSendingComplete(timeout);
    }

    virtual void SetPriority(int priority)
    {
        m_priority = priority;
    }
private:
    /// @brief �Ƿ���server�˵�
    static bool IsServerEndPoint(const std::string& endpoint);

    /// @brief Ѱ��session���������򴴽�
    Session* FindOrMakeSession(const std::string& local_endpoint,
            const std::string& remote_endpoint);

private:
    SimpleMutex m_mutex;
    netframe::NetFrame m_netframe;               ///< ʵ�ʵ�������
    SessionManager m_session_manager;            ///< �Ự������
    std::set<std::string> m_listen_endpoints;    ///< �����Ķ˵��б�
    Rpc::Channel::EventHandler* m_event_handler; ///< �¼�������ָ��

    unsigned int m_connect_timeout;              ///< ��ʱ���ã���
    unsigned int m_max_connect_number;           ///< �����������
    unsigned int m_max_packet_length;            ///< ������
    int m_priority;
};

}

#endif
