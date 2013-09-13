#ifndef  XCUBE_DISTRIBUTE_LOCK_EVENT_MASK__H__
#define  XCUBE_DISTRIBUTE_LOCK_EVENT_MASK__H__
#include <common/base/stdint.h>

namespace distribute_lock
{
    enum ZKEventMask
    {
        EVENT_MASK_NONE                                 = 0x0000, ///< 无感兴趣事件
        EVENT_MASK_ATTR_CHANGED                         = 0x0001, ///< 属性改变事件
        EVENT_MASK_CHILD_CHANGED                        = 0x0002, ///< 孩子节点属性的改变
        EVENT_MASK_LOCK_RELEASE                         = 0x0004, ///< 锁失效
        EVENT_MASK_DATA_CHANGED                         = 0x0008, ///< 值改变事件
        EVENT_MASK_LOCK_ACQUIRED                        = 0x0010, ///< 加锁事件
        EVENT_MASK_SESSION_EXPIRED                      = 0x0020  ///< session失效事件
    };
}/// distribute_lock


#endif  // XCUBE_DISTRIBUTE_LOCK__DATA_STRUCT_H__
