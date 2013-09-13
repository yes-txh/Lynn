//  min_heap_obj.h
//  wookin
//  2010-06-03
//
// 1:封装min_heap对象
//
// ////////////////////////////////////////////////////////////
#ifndef COMMON_BASELIB_SVRPUBLIB_MIN_HEAP_OBJ_H_
#define COMMON_BASELIB_SVRPUBLIB_MIN_HEAP_OBJ_H_

#include "common/baselib/svrpublib/min_heap.h"
#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

//
// 1:封装min_heap对象
// 2:T结构中必须包含一个成员变量为struct min_event m_event;
//
template <typename T>
class CMinHeapObj_T {
public:

    //
    // 压入一个节点(按超时时长进行排序, 超时最长在最后面, 最小堆排序)
    // tduration:可持续的生存期
    //
    void Push(T* item, struct timeval* duration) {
        if (item) {
            // 强行初始化
            struct min_event* ev = &(item->m_event);
            min_heap_elem_init(ev);
            ev->t_duration = *duration;
            ev->parent = reinterpret_cast<void*>(item);

            // Push
            min_heap_push(&m_heap, ev);
        }
    }

    //
    // 取出一个节点(取顶部节点)
    //
    T*  Pop(void) {
        T* item = 0;
        struct min_event* ev = min_heap_pop(&m_heap);
        if (ev) {
            item = reinterpret_cast<T*>(ev->parent);
            ev->parent = 0; // 清空
        }

        return item;
    }

    //
    // GetTopPtr(获取顶点节点的指针, 并不取出)
    // GetTopRefPtr
    //
    T*  GetTopRefPtr(void) {
        T* item = 0;
        struct min_event* ev = min_heap_top(&m_heap);
        if (ev) {
            item = reinterpret_cast<T*>(ev->parent);
        }

        return item;
    }

    //
    // 从堆中删除某个节点(如果存在)
    //
    void Erase(T* item) {
        if (item) {
            min_heap_erase(&m_heap, &(item->m_event));
            item->m_event.parent = 0;  //  清空
        }
    }

    //
    // 获取堆中元素个数
    //
    uint32_t GetSize(void) {
        return min_heap_size(&m_heap);
    }

    //
    // 获取剩余时间, 如果剩余时间不存在，在ptm里面sec, usec都会为0
    //
    void GetRemainTime(const T* item, struct timeval* tv) {
        if (item) {
            // a
            // 流逝的时间 t_lost = t_now - t_begin
            //
            struct timeval t_now;
            lite_gettimeofday(&t_now, 0);
            const struct min_event* pEvent = &item->m_event;
            int64_t t_lost_a =
                int64_t((t_now.tv_sec - pEvent->t_begin.tv_sec)*1000000)
                + int64_t(t_now.tv_usec)
                - int64_t(pEvent->t_begin.tv_usec);

            //
            // 剩余的时间 t_remain = t_duration - t_lost
            //
            int64_t t_remain_a = int64_t(pEvent->t_duration.tv_sec)*1000000 +
                                 int64_t(pEvent->t_duration.tv_usec) -
                                 int64_t(t_lost_a);

            t_remain_a = t_remain_a >0 ? t_remain_a : 0;
            tv->tv_sec = int32_t(t_remain_a/1000000);
            tv->tv_usec = int32_t(t_remain_a%1000000);
        }
    }

    //
    // 构造, 析构函数
    //
    CMinHeapObj_T() {
        min_heap_ctor(&m_heap);
    }

    virtual ~CMinHeapObj_T() {
        min_heap_dtor(&m_heap);
    }

private:
    struct min_heap_t m_heap;
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_MIN_HEAP_OBJ_H_
