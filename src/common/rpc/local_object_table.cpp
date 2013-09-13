#include "common/rpc/local_object_table.hpp"
#include <common/rpc/stub.hpp>
#include <common/rpc/scheduler.hpp>

namespace Rpc
{

// LocalObjectTable ��ʵ��

/// �Ѷ���ע�ᵽ����
/// @param object Ҫ����Ķ���
/// @param replace ��� ID �Ķ����Ѵ��ڣ��Ƿ������滻
/// @return �Ƿ�ɹ�ע��
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

/// �ӱ���ע������
/// @param object Ҫע���Ķ���
/// @return �Ƿ�ɹ�ע�������ɹ���Ψһԭ���Ƕ����ڱ��У�
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
