#ifndef  XCUBE_DISTRIBUTE_LOCK_DATA_STRUCT_H__
#define  XCUBE_DISTRIBUTE_LOCK_DATA_STRUCT_H__

#include <string>

namespace distribute_lock
{
struct Attr
{
    Attr()
    {
        name    = "";
        val     = "";
    }
    std::string name;               ///< 节点名字
    std::string val;                ///< 节点数据
};

}// namespace


#endif  // XCUBE_DISTRIBUTE_LOCK_DATA_STRUCT_H__
