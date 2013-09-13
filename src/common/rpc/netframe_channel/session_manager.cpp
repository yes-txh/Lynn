/// @date 04/29/2010
/// @author jeremychen (chenzheng)

#include <time.h>

#include "common/system/concurrency/scoped_locker.hpp"
#include "common/rpc/netframe_channel/session.hpp"
#include "common/rpc/netframe_channel/session_manager.hpp"

namespace Rpc
{

/// @brief ��ӻỰ���Ự������
/// @param session �Ựָ��
void SessionManager::AddSession(Session* session)
{
    MutexLocker locker(m_mutex);
    const std::string& session_name = session->GetName();
    assert(m_session_table.find(session_name) == m_session_table.end());
    m_session_table[session_name] = session;
}

/// @brief  ����end point���һỰ
/// @param  local_endpoint �ͻ��˵�EndPoint
/// @param  remote_endpoint ��������EndPoint
/// @return �ҵ������棬δ�ҵ����ؼ�
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

/// @brief  ����SocketID���һỰ
/// @param  sockid SocketID
/// @return ���ض�ӦSession
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

/// @brief  ͳ�Ƶ�ǰ�Ự����
/// @return ���ص�ǰ�Ự�������еĻỰ����
size_t SessionManager::GetSessionNumber()
{
    MutexLocker locker(m_mutex);
    return m_session_table.size();
}

/// @brief  ��ȡ�ͻ���EndPoint��Ӧ��SocketID
/// @param  local_endpoint �ͻ��˵�EndPoint
/// @param  remote_endpoint ��������EndPoint
/// @return ����Socket ID
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

/// @brief ���ֻỰ������״̬
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
    /// ֻ��session�Ͽ���ʱ��ɾ��
    if (session && session->GetStatus() == Session::Status_Disconnected)
    {
        DeleteSession(session);
    }
}

/// @brief ɾ��EndPoint��Ӧ�ĻỰ
/// @param session �Ựָ��
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

/// @brief ������������е��д��ڵ�session
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
