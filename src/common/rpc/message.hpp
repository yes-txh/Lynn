#ifndef RPC_MESSAGE_HPP
#define RPC_MESSAGE_HPP

/// @file
/// 定义了 RPC 过程中需要的各种消息。


#define _CRT_SECURE_NO_WARNINGS 1

#include <string.h>
#include <common/base/stdint.h>
#include <common/rpc/types.hpp>

/// 消息头可能落在任意地址上，因此要处理对齐问题。
/// VC 用 UNALIGNED 宏来定义不对齐的结构
/// UNALIGNED 在对齐敏感的体系结构上展开为__unaligned 关键字，否则为空
#ifndef _MSC_VER
#define UNALIGNED
#endif

/// gcc 用 __attribute__((packed)) 来定义不对齐的结构
#ifdef __GNUC__
#define UNALIGNED_FIELD __attribute__((packed))
#else
#define UNALIGNED_FIELD
#endif

namespace Rpc
{

const uint16_t ByteOrderTester = 0x1234;

/// RPC 消息类型定义
enum MessageType
{
    MessageType_LogIn = 1,          ///< 客户端登陆
    MessageType_LogInAck = 2,       ///< 服务器端回应登录
    MessageType_LogOut = 3,         ///< 客户端退出
    MessageType_Invoke = 4,         ///< 调用消息
    MessageType_Return = 5,         ///< 返回消息
    MessageType_Max,                ///< 最大类型
};

/// 所有 RPC 消息的公共头
struct MessageHeader
{
    char Signature[4];                      ///< 签名, 必须等于 "RPC\0"
    uint16_t ByteOrder UNALIGNED_FIELD;     ///< 通常应该等于 ByteOrderTester，如果不是，RPC 进行转换
    uint16_t Type UNALIGNED_FIELD;          ///< 消息类型
    uint16_t HeaderLength UNALIGNED_FIELD;  ///< 实际头的长度，不同的类型头部长度不一样
    uint32_t Length UNALIGNED_FIELD;        ///< 整个消息的长度
protected:
    MessageHeader(MessageType type, size_t header_length, size_t length);
};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable :4200) // 消除数组大小为 0 的警告
#endif
/// RPC 登录消息
struct LogInMessageHeader : MessageHeader
{
    /// 登录端自己的标识符，变长数组，\0 结尾，此处为占位用的 flexible 数组，
    /// 多出来的部分要自己分配。
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

/// RPC 登录的回复
struct LogInAckMessageHeader : MessageHeader
{
    Status_t Status UNALIGNED_FIELD;
    LogInAckMessageHeader(size_t header_length, size_t length, Status_t status);
};

/// 调用和返回消息的公共部分
struct CommonMessageHeader : MessageHeader
{
    InvokeId_t InvokeId UNALIGNED_FIELD;    ///< 调用 ID
    int32_t MethodId UNALIGNED_FIELD;       ///< 方法 ID
protected:
    CommonMessageHeader(
        MessageType type,
        size_t header_length,
        InvokeId_t invoke_id,
        int method_id,
        size_t length
    );
};

/// RPC 调用的消息头
struct InvokeMessageHeader : CommonMessageHeader
{
    int32_t Pid UNALIGNED_FIELD;                    ///< 进程 ID
    ObjectId_t ObjectId UNALIGNED_FIELD;            ///< 对象 ID
    char MethodName[MaxMethodNameLength + 1];       ///< 方法名
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

/// RPC 调用返回的消息头
struct ReturnMessageHeader : CommonMessageHeader
{
    int32_t Status;                     ///< 完成状态
public:
    ReturnMessageHeader(
        InvokeId_t invoke_id,           ///< 调用 ID
        int method_id,                  ///< 方法 ID
        Status_t status,                ///< 状态码
        size_t body_length              ///< 消息体长度
    );
};

} // end namespace Rpc

#endif // RPC_MESSAGE_HPP
