#ifndef RPC_LOCAL_OBJECT_TABLE_HPP
#define RPC_LOCAL_OBJECT_TABLE_HPP

#include <common/rpc/types.hpp>
#include <map>

namespace Rpc
{

class Stub;

/// 本地对象表，保存 ID -> 对象的映射
/// 所有在远程被用到的对象都在一个统一的表里
class LocalObjectTable
{
public:
    /// 处理调用消息
    Status_t Dispatch(
        const std::string& loacl_endpoint,  ///< 本地的端点
        const std::string& remote_endpoint, ///< 调用者的端点
        InvokeId_t invoke_id,               ///< 调用 ID
        ObjectId_t object_id,               ///< 对象 ID
        const char* name,                   ///< 方法名
        int method_id,                      ///< 方法 ID
        const void* message,                ///< 消息缓冲区
        size_t size                         ///< 消息长度
    );

    /// 把对象注册到表中
    /// @param object 要放入的对象
    /// @param replace 如果 ID 的对象已存在，是否允许替换
    /// @return 是否成功注册
    bool Register(const Stub* object, bool replace = false);

    /// 从表中注销对象
    /// @param object 要注销的对象
    /// @return 是否成功注销（不成功的唯一原因是对象不在表中）
    bool Unregister(const Stub* object);

    /// 函数是否支持嵌套
    /// @param object_id stub对象id
    /// @param name 函数名
    /// @param method_id 函数在表项中的id
    bool IsNestableMethod(ObjectId_t object_id, const char* name, int method_id);
private:
    std::map<ObjectId_t, Stub*> m_ObjectMap;
    SimpleMutex m_Mutex;
};

} // namespace Rpc

#endif//RPC_LOCAL_OBJECT_TABLE_HPP

