#include <common/rpc/server.hpp>
#include <common/rpc/stub.hpp>

namespace Rpc
{

bool Server::AddNamedObject(const char *name, Stub *object, bool replace)
{
    ObjectId_t oid = object->RpcObjectId();
    std::pair<NamedObjectMap::iterator, bool> p = m_NamedObjects.insert(std::make_pair(name, oid));
    if (p.second)
        return true;
    if (replace)
    {
        p.first->second = oid;
        return true;
    }
    return false;
}

bool Server::RemoveNamedObject(const char *name)
{
    return m_NamedObjects.erase(name) > 0;
}

bool Server::Run()
{
    return true;
}

}
