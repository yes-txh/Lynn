/// @brief  session manager
/// @author jeremychen (chenzheng)

#ifndef COMMON_RPC_SESSION_MANAGER_HPP
#define COMMON_RPC_SESSION_MANAGER_HPP

#include "common/rpc/types.hpp"
#include "common/base/closure.h"
#include "common/base/stdext/hash_map.hpp"
#include "common/system/timer/timer_manager.hpp"
#include "common/rpc/netframe_channel/session.hpp"

namespace Rpc
{

class SessionManager
{
public:
    SessionManager()
    {
        /// 注册周期性定时器，维持连接
        Closure<void, uint64_t>* closure = NewPermanentClosure(this,
                &SessionManager::Maintain);
        m_timer_manager.AddPeriodTimer(1000, closure);
    }

    ~SessionManager();

    /// @brief 添加新会话
    void AddSession(Session* session);

    /// @brief 查找会话
    Session* FindByEndPoint(const std::string& local_endpoint,
            const std::string& remote_endpoint);

    /// @brief 依据socket id查找会话
    Session* FindBySocketId(int64_t socket_id);

    /// @brief 获取会话数
    size_t GetSessionNumber();

    /// @brief 获取某个会话的socket id
    int64_t GetSocketID(const std::string& local_endpoint,
            const std::string& remote_endpoint);

    /// @brief 添加session关闭的定时器，所有session均延迟删除，以备重新使用
    bool AddSessionCloseTimer(Session* session)
    {
        Closure<void, uint64_t>* closure = NewClosure(
                this, &SessionManager::OnSessionClose, session);
        m_timer_manager.AddOneshotTimer(1000, closure);
        return true;
    }

private:
    /// @brief 保持所有会话
    void Maintain(uint64_t timer_id);

    /// @brief session关闭时候的回调
    void OnSessionClose(Session* session, uint64_t unuse_param);

    /// @brief 删除会话，不公开使用，使用定时器延迟删除
    void DeleteSession(Session* session);

private:
    typedef stdext::hash_map<std::string, Session*> SessionTable;
    SessionTable m_session_table;   ///< Session表
    SimpleMutex m_mutex;            ///< 访问Session表的mutex
    TimerManager m_timer_manager;   ///< 定时器管理
};

} // end namespace Rpc

#endif
