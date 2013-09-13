#ifndef RPC_MESSAGE_HPP
#define RPC_MESSAGE_HPP

/// @file
/// ������ RPC ��������Ҫ�ĸ�����Ϣ��


#define _CRT_SECURE_NO_WARNINGS 1

#include <string.h>
#include <common/base/stdint.h>
#include <common/rpc/types.hpp>

/// ��Ϣͷ�������������ַ�ϣ����Ҫ����������⡣
/// VC �� UNALIGNED �������岻����Ľṹ
/// UNALIGNED �ڶ������е���ϵ�ṹ��չ��Ϊ__unaligned �ؼ��֣�����Ϊ��
#ifndef _MSC_VER
#define UNALIGNED
#endif

/// gcc �� __attribute__((packed)) �����岻����Ľṹ
#ifdef __GNUC__
#define UNALIGNED_FIELD __attribute__((packed))
#else
#define UNALIGNED_FIELD
#endif

namespace Rpc
{

const uint16_t ByteOrderTester = 0x1234;

/// RPC ��Ϣ���Ͷ���
enum MessageType
{
    MessageType_LogIn = 1,          ///< �ͻ��˵�½
    MessageType_LogInAck = 2,       ///< �������˻�Ӧ��¼
    MessageType_LogOut = 3,         ///< �ͻ����˳�
    MessageType_Invoke = 4,         ///< ������Ϣ
    MessageType_Return = 5,         ///< ������Ϣ
    MessageType_Max,                ///< �������
};

/// ���� RPC ��Ϣ�Ĺ���ͷ
struct MessageHeader
{
    char Signature[4];                      ///< ǩ��, ������� "RPC\0"
    uint16_t ByteOrder UNALIGNED_FIELD;     ///< ͨ��Ӧ�õ��� ByteOrderTester��������ǣ�RPC ����ת��
    uint16_t Type UNALIGNED_FIELD;          ///< ��Ϣ����
    uint16_t HeaderLength UNALIGNED_FIELD;  ///< ʵ��ͷ�ĳ��ȣ���ͬ������ͷ�����Ȳ�һ��
    uint32_t Length UNALIGNED_FIELD;        ///< ������Ϣ�ĳ���
protected:
    MessageHeader(MessageType type, size_t header_length, size_t length);
};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable :4200) // ���������СΪ 0 �ľ���
#endif
/// RPC ��¼��Ϣ
struct LogInMessageHeader : MessageHeader
{
    /// ��¼���Լ��ı�ʶ�����䳤���飬\0 ��β���˴�Ϊռλ�õ� flexible ���飬
    /// ������Ĳ���Ҫ�Լ����䡣
    char Identifier[1];
    LogInMessageHeader(size_t header_length, size_t length);
};

struct LogOutMessageHeader : MessageHeader
{
    char Identifier[1];
    LogOutMessageHeader(size_t header_length, size_t length);
};


#ifdef _MSC_VER
#pragma warning(pop)
#endif

/// RPC ��¼�Ļظ�
struct LogInAckMessageHeader : MessageHeader
{
    Status_t Status UNALIGNED_FIELD;
    LogInAckMessageHeader(size_t header_length, size_t length, Status_t status);
};

/// ���úͷ�����Ϣ�Ĺ�������
struct CommonMessageHeader : MessageHeader
{
    InvokeId_t InvokeId UNALIGNED_FIELD;    ///< ���� ID
    int32_t MethodId UNALIGNED_FIELD;       ///< ���� ID
protected:
    CommonMessageHeader(
        MessageType type,
        size_t header_length,
        InvokeId_t invoke_id,
        int method_id,
        size_t length
    );
};

/// RPC ���õ���Ϣͷ
struct InvokeMessageHeader : CommonMessageHeader
{
    int32_t Pid UNALIGNED_FIELD;                    ///< ���� ID
    ObjectId_t ObjectId UNALIGNED_FIELD;            ///< ���� ID
    char MethodName[MaxMethodNameLength + 1];       ///< ������
public:
    InvokeMessageHeader(
        int pid,
        ObjectId_t object_id,
        InvokeId_t invoke_id,
        int method_id,
        const char* method_name,
        size_t length
    );
};

/// RPC ���÷��ص���Ϣͷ
struct ReturnMessageHeader : CommonMessageHeader
{
    int32_t Status;                     ///< ���״̬
public:
    ReturnMessageHeader(
        InvokeId_t invoke_id,           ///< ���� ID
        int method_id,                  ///< ���� ID
        Status_t status,                ///< ״̬��
        size_t body_length              ///< ��Ϣ�峤��
    );
};

} // end namespace Rpc

#endif // RPC_MESSAGE_HPP
