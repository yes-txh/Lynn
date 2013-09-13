#include "common/rpc/local_object_table.hpp"
#include <common/rpc/stub.hpp>
#include <common/rpc/scheduler.hpp>

namespace Rpc
{

// LocalObjectTable 的实现

/// 把对象注册到表中
/// @param object 要放入的对象
/// @param replace 如果 ID 的对象已存在，是否允许替换
/// @return 是否成功注册
bool LocalObjectTable::Register(const Stub* object, bool replace)
{
    MutexLocker lock(m_Mutex);
    ObjectId_t id = object->RpcObjectId();
    std::map<ObjectId_t, Stub*>::iterator it = m_ObjectMap.find(id);
    if (it == m_ObjectMap.end())
    {
        m_ObjectMap[id] = const_cast<Stub*>(object);
        return true;
    }
    else if (replace)
    {
        it->second = const_cast<Stub*>(object);
        return true;
    }
    return false;
}

/// 从表中注销对象
/// @param object 要注销的对象
/// @return 是否成功注销（不成功的唯一原因是对象不在表中）
bool LocalObjectTable::Unregister(const Stub* object)
{
    MutexLocker lock(m_Mutex);
    return m_ObjectMap.erase(object->RpcObjectId()) == 1;
}

bool LocalObjectTable::IsNestableMethod(ObjectId_t object_id, const char* name, int method_id)
{
    MutexLocker lock(m_Mutex);
    std::map<ObjectId_t, Stub*>::iterator it = m_ObjectMap.find(object_id);
    if (it != m_ObjectMap.end())
    {
        return it->second->IsNestableMethod(name, method_id);
    }
    return false;
}

Status_t LocalObjectTable::Dispatch(
    const std::string& local_endpoint,
    const std::string& remote_endpoint,
    InvokeId_t invoke_id,
    ObjectId_t object_id,
    const char* name,
    int method_id,
    const void* message,
    size_t size
)
{
    Stub * stub = NULL;
    {
        MutexLocker lock(m_Mutex);
        std::map<ObjectId_t, Stub*>::iterator it = m_ObjectMap.find(object_id);
        if (it != m_ObjectMap.end())
        {
            assert(it->second != NULL);
            stub = it->second;
        }
    }

    if (stub != NULL)
    {
        return stub->Dispatch(local_endpoint, remote_endpoint, invoke_id, name, method_id, message, size);
    }
    else
    {
        Scheduler::Instance().ErrorReturn(local_endpoint, remote_endpoint, invoke_id, method_id, Status_ObjectNotFound);
        return Status_ObjectNotFound;
    }
}

} // end namespace Rpc
