/// @date 04/29/2010
/// @author jeremychen (chenzheng)

#include <time.h>

#include "common/system/concurrency/scoped_locker.hpp"
#include "common/rpc/netframe_channel/session.hpp"
#include "common/rpc/netframe_channel/session_manager.hpp"

namespace Rpc
{

/// @brief 添加会话到会话管理器
/// @param session 会话指针
void SessionManager::AddSession(Session* session)
{
    MutexLocker locker(m_mutex);
    const std::string& session_name = session->GetName();
    assert(m_session_table.find(session_name) == m_session_table.end());
    m_session_table[session_name] = session;
}

/// @brief  根据end point查找会话
/// @param  local_endpoint 客户端的EndPoint
/// @param  remote_endpoint 服务器的EndPoint
/// @return 找到返回真，未找到返回假
Session* SessionManager::FindByEndPoint(const std::string& local_endpoint,
        const std::string& remote_endpoint)
{
    std::string session_name = Session::GetSessionName(
            local_endpoint, remote_endpoint);

    MutexLocker locker(m_mutex);
    SessionTable::const_iterator iter = m_session_table.find(session_name);
    if (iter != m_session_table.end())
    {
        return iter->second;
    }
    return NULL;
}

/// @brief  依据SocketID查找会话
/// @param  sockid SocketID
/// @return 返回对应Session
Session* SessionManager::FindBySocketId(int64_t socket_id)
{
    MutexLocker locker(m_mutex);
    SessionTable::const_iterator iter = m_session_table.begin();
    for (; iter != m_session_table.end(); ++iter)
    {
        Session* session = iter->second;
        if (session->GetSocketID() == socket_id &&
            session->GetStatus() != Session::Status_Disconnected)
        {
            return session;
        }
    }
    return NULL;
}

/// @brief  统计当前会话个数
/// @return 返回当前会话管理器中的会话个数
size_t SessionManager::GetSessionNumber()
{
    MutexLocker locker(m_mutex);
    return m_session_table.size();
}

/// @brief  获取客户端EndPoint对应的SocketID
/// @param  local_endpoint 客户端的EndPoint
/// @param  remote_endpoint 服务器的EndPoint
/// @return 返回Socket ID
int64_t SessionManager::GetSocketID(const std::string& local_endpoint,
        const std::string& remote_endpoint)
{
    MutexLocker locker(m_mutex);
    Session* session = FindByEndPoint(local_endpoint, remote_endpoint);
    if (session)
    {
        return session->GetSocketID();
    }
    return -1;
}

/// @brief 保持会话长连接状态
void SessionManager::Maintain(uint64_t timer_id)
{
    MutexLocker locker(m_mutex);
    SessionTable::iterator iter = m_session_table.begin();
    for (; iter != m_session_table.end(); ++iter)
    {
        iter->second->Maintain();
    }
}

void SessionManager::OnSessionClose(Session* session, uint64_t unuse_param)
{
    /// 只在session断开的时候删除
    if (session && session->GetStatus() == Session::Status_Disconnected)
    {
        DeleteSession(session);
    }
}

/// @brief 删除EndPoint对应的会话
/// @param session 会话指针
void SessionManager::DeleteSession(Session* session)
{
    MutexLocker locker(m_mutex);
    const std::string& session_name = session->GetName();
    SessionTable::iterator iter = m_session_table.find(session_name);
    if (iter != m_session_table.end())
    {
        delete iter->second;
        iter->second = NULL;
        m_session_table.erase(iter);
    }
}

/// @brief 析构，清除所有的尚存在的session
SessionManager::~SessionManager()
{
    MutexLocker locker(m_mutex);
    SessionTable::iterator iter = m_session_table.begin();
    while (iter != m_session_table.end())
    {
        delete iter->second;
        ++iter;
    }
    m_session_table.clear();
}

} // end namespace Rpc
