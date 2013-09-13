#ifndef RPC_REMOTE_OBJECT_TABLE_HPP
#define RPC_REMOTE_OBJECT_TABLE_HPP

#include <string>
#include <map>
#include <common/rpc/types.hpp>
#include <common/rpc/proxy.hpp>

namespace Rpc
{

/// 远程对象表
class RemoteObjectTable
{
public:
    typedef std::map<std::string, std::map<ObjectId_t, Proxy*> > ObjectTable;
};

} // end namespace Rpc

#endif // RPC_REMOTE_OBJECT_TABLE_HPP
