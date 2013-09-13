/// @file
/// ʵ��  Rpc::Message

#include <common/rpc/message.hpp>

namespace Rpc
{

MessageHeader:: MessageHeader(MessageType type, size_t header_length, size_t length)
    : ByteOrder(ByteOrderTester), Type(type), HeaderLength(header_length), Length(length)
{
    memcpy(Signature, "RPC", sizeof(Signature));
}

LogInMessageHeader::LogInMessageHeader(size_t header_length, size_t length)
    : MessageHeader(MessageType_LogIn, header_length, length)
{
}

LogInAckMessageHeader::LogInAckMessageHeader(size_t header_length, size_t length, Status_t status)
    : MessageHeader(MessageType_LogInAck, header_length, length)
{
    Status = status;
}

LogOutMessageHeader::LogOutMessageHeader(size_t header_length, size_t length)
    : MessageHeader(MessageType_LogOut, header_length, length)
{
}

CommonMessageHeader::CommonMessageHeader(
    MessageType type,
    size_t header_length,
    InvokeId_t invoke_id,
    int method_id,
    size_t length
) :
    MessageHeader(type, header_length, length),
    InvokeId(invoke_id),
    MethodId(method_id)
{
}

InvokeMessageHeader::InvokeMessageHeader(
    int pid,
    ObjectId_t object_id,
    InvokeId_t invoke_id,
    int method_id,
    const char* method_name,
    size_t length
) :
    CommonMessageHeader(MessageType_Invoke, sizeof(*this), invoke_id, method_id, length), Pid(pid), ObjectId(object_id)
{
    if (method_id >= 0)
    {
        MethodName[0] = '\0';
    }
    else
    {
        strncpy(MethodName, method_name, MaxMethodNameLength);
        MethodName[MaxMethodNameLength] = '\0';
    }
}

ReturnMessageHeader::ReturnMessageHeader(
    InvokeId_t invoke_id,   ///< ���� ID
    int method_id,          ///< ���� ID
    Status_t status,        ///< ״̬��
    size_t body_length      ///< ��Ϣ�峤��
):
    CommonMessageHeader(MessageType_Return, sizeof(*this), invoke_id, method_id, body_length),
    Status(status)
{
    Type = MessageType_Return;
    HeaderLength = sizeof(*this);
}

/// ������Ϣͷ�Ĵ�С�����ڼ������л�ǰԤ���ռ�
extern const size_t InvokeMessageHeaderSize = sizeof(InvokeMessageHeader);

/// ������Ϣͷ�Ĵ�С�����ڼ������л�ǰԤ���ռ�
extern const size_t ReturnMessageHeaderSize = sizeof(ReturnMessageHeader);

} // end namespace Rpc
