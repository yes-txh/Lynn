#ifndef RPC_TYPES_HPP_INCLUDED
#define RPC_TYPES_HPP_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/// Base definations for RPC

#include <stddef.h>
#include <errno.h>
#include <stdexcept>
#include <string>

#include <common/base/stdint.h>
#include <common/rpc/config.hpp>

namespace Rpc
{

/// ���� ID ����
typedef long long ObjectId_t;

/// ���� ID ����
typedef long long InvokeId_t;

/// ���֧�ֵ�Ԥ�������ĸ�����ID ��ǰ��ô���λ������˫��Ԥ��Լ����
const ObjectId_t MaxPredefinedObjectId = 0x10000;

const size_t MaxMethodNameLength = 31;

/// ���֧�ֵĲ�������
const size_t MaxParameterCount = 8;

/// ��������������
class Buffer
{
public:
    Buffer(const void* ptr, size_t size):
        m_Ptr(const_cast<void*>(ptr)), m_Size(size)
    {
    }
    Buffer():
        m_Ptr(NULL), m_Size(0)
    {}
    void* Address()
    {
        return m_Ptr;
    }
    const void* Address() const
    {
        return m_Ptr;
    }
    size_t Size() const
    {
        return m_Size;
    }
private:
    void* m_Ptr;
    size_t m_Size;
};

/// RPC ��״̬�붨��
enum Status_t
{
    Status_Success,                         ///< �����ɹ�
    Status_Pending,                         ///< ������δ���
    Status_Timeout,                         ///< ��ʱ
    Status_Canceled,                        ///< ����������ȡ��
    Status_MethodFailure,                   ///< RPC �ɹ������Ǻ�������ִ�д����з���ֵ���޳�����
    Status_InvalidObject,                   ///< ��Ч���� ID
    Status_InvalidEndPoint,                 ///< ��Ч EndPoint
    Status_EndPointInUse,                   ///< EndPoint �ѱ�ʹ��
    Status_ObjectNotFound,                  ///< �����ڶԷ����̲�����
    Status_MethodNotFound,                  ///< ָ���ķ���������
    Status_ProcessNotFound,                 ///< Ŀ����̲����ڣ������Ѿ���ֹ��
    Status_InvalidInvokeId,                 ///< ����δ�ҵ��������ѱ�ɾ��
    Status_ConnectionError,                 ///< ���Ӵ���
    Status_LocalInvalidInputArguments,      ///< ���ش������������Ч
    Status_LocalInvalidOutputArguments,     ///< ���ش������������Ч
    Status_LocalInvalidReturnValue,         ///< ���ش��󣺷���ֵ��Ч
    Status_RemoteInvalidInputArguments,     ///< Զ�̴������������Ч
    Status_RemoteInvalidOutputArguments,    ///< Զ�̴������������Ч
    Status_RemoteInvalidReturnValue,        ///< Զ�̴��󣺷���ֵ��Ч
    Statux_Max                              ///< ��Ϊ�Զ�����״̬��ĸ����ã�����ʵ�ʵ�״̬�롣
};

/// ״̬���ǲ���һ������
inline bool Status_IsError(Status_t status)
{
    return status != Status_Success && status != Status_Pending;
}

/// ״̬���ǲ���һ�����أ���ͨѶ������
inline bool Status_IsLocalError(Status_t status)
{
    return Status_IsError(status) && status != Status_ConnectionError;
}

/// ���״̬���Ӧ���ַ�����
/// @param status
/// @undefined_string δ����ʱ���ص�ֵ
inline const char* StatusString(Status_t status, const char* undefined_string = "<Undefined status code>")
{
    switch (status)
    {
#define StatusString_MAKE_CASE(Name) case Status_##Name: return #Name
        StatusString_MAKE_CASE(Success);
        StatusString_MAKE_CASE(Pending);
        StatusString_MAKE_CASE(Timeout);
        StatusString_MAKE_CASE(Canceled);
        StatusString_MAKE_CASE(MethodFailure);
        StatusString_MAKE_CASE(InvalidObject);
        StatusString_MAKE_CASE(InvalidEndPoint);
        StatusString_MAKE_CASE(EndPointInUse);
        StatusString_MAKE_CASE(ObjectNotFound);
        StatusString_MAKE_CASE(MethodNotFound);
        StatusString_MAKE_CASE(ProcessNotFound);
        StatusString_MAKE_CASE(InvalidInvokeId);
        StatusString_MAKE_CASE(ConnectionError);
        StatusString_MAKE_CASE(LocalInvalidInputArguments);
        StatusString_MAKE_CASE(LocalInvalidOutputArguments);
        StatusString_MAKE_CASE(LocalInvalidReturnValue);
        StatusString_MAKE_CASE(RemoteInvalidInputArguments);
        StatusString_MAKE_CASE(RemoteInvalidOutputArguments);
        StatusString_MAKE_CASE(RemoteInvalidReturnValue);
#undef StatusString_MAKE_CASE
    default:
        break;
    }
    return undefined_string;
}

/// @brief �첽��ɵĻص�����ԭ��
/// @param context ����������Ҫ����������Ϣ
/// @param status ���յ�״̬
/// @param token �첽�������
/// @param param ������������
typedef void (*AsyncCallback)(void* context, Status_t status, void* param);

} // end namespace Rpc

#endif // RPC_TYPES_HPP_INCLUDED
