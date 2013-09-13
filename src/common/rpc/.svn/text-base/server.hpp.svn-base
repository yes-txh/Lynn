#ifndef RPC_SERVER_HPP_INCLUDED
#define RPC_SERVER_HPP_INCLUDED

#include <common/rpc/types.hpp>

#include <string>
#include <map>

namespace Rpc
{

class Stub;

/// RPC 服务器类
class Server
{
public:
    /// 添加命名对象，可以在客户端通过名字得到。
    /// @param name 对象名称
    /// @param object 对象存根的地址
    /// @param replace 如果对象已存在，是否替换
    /// @return 是否成功
    bool AddNamedObject(const char* name, Stub* object, bool replace = false);

    /// 删除命名对象
    /// @param name 对象名称
    /// @return 是否成功
    bool RemoveNamedObject(const char* name);

    ///
    bool Run();
private:
    typedef std::map<std::string, ObjectId_t> NamedObjectMap;
    NamedObjectMap m_NamedObjects;
};

}

#endif // RPC_SERVER_HPP_INCLUDED

