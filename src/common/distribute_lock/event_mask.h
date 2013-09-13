#ifndef  XCUBE_DISTRIBUTE_LOCK_EVENT_MASK__H__
#define  XCUBE_DISTRIBUTE_LOCK_EVENT_MASK__H__
#include <common/base/stdint.h>

namespace distribute_lock
{
    enum ZKEventMask
    {
        EVENT_MASK_NONE                                 = 0x0000, ///< �޸���Ȥ�¼�
        EVENT_MASK_ATTR_CHANGED                         = 0x0001, ///< ���Ըı��¼�
        EVENT_MASK_CHILD_CHANGED                        = 0x0002, ///< ���ӽڵ����Եĸı�
        EVENT_MASK_LOCK_RELEASE                         = 0x0004, ///< ��ʧЧ
        EVENT_MASK_DATA_CHANGED                         = 0x0008, ///< ֵ�ı��¼�
        EVENT_MASK_LOCK_ACQUIRED                        = 0x0010, ///< �����¼�
        EVENT_MASK_SESSION_EXPIRED                      = 0x0020  ///< sessionʧЧ�¼�
    };
}/// distribute_lock


#endif  // XCUBE_DISTRIBUTE_LOCK__DATA_STRUCT_H__
