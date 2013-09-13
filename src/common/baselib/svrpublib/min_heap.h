//  min_heap.h
//  wookin
//  2010-5-5
//
// 1:�޸���libevent libevent-2.0.4-alpha, �����޸�����:
//
// 2:��ԭ��realloc, free�޸�Ϊnew, delete
//
// 3:��struct event�޸�Ϊstruct min_event
//
// 4:��ԭ����ʱʱ��t_timeout�޸�Ϊ:
//   a:ÿ�μ����ʱʱ���е�ǰʱ��t_begin, ��ʱʱ��t_duration.
//   b:ÿ�αȽϸ��ڵ㳬ʱ��Сʱ��ȡ��ǰʱ��t_now, ��ʱ������
//     t_lost = t_now-t_begin
//   c:�ڵ�ʣ��������ڼ�Ϊ��ʱʱ��t_remain = t_duration-t_lost,
//     (t_remainֻȡ���ڵ���0��ֵ, С��0˵���Ѿ���ʱ)
//
// 5:�����û��Զ������ݽṹ, �洢˽������(user_data)
//
// 6:ʱ��ȡֵ, ��n��ȡ��time(0), С��1�벿�� = gettimeofday()%1000000
//
// ////////////////////////////////////////////////////////////

// //////////////////////////////////////
#ifndef COMMON_BASELIB_SVRPUBLIB_MIN_HEAP_H_
#define COMMON_BASELIB_SVRPUBLIB_MIN_HEAP_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

//
// min_event
//
struct min_event {
    struct timeval t_begin;     //  ��ʼ�����ʱ��(��ʼ���뵽min_heap��ʱ��)
    struct timeval t_duration;  //  �ɳ�����������

    union {
        int32_t min_heap_idx;
    } ev_timeout_pos;

    // //////////////////
    //
    // �û��Զ������ݽṹ, �û��������ɶ���ýṹ
    //
    typedef union {
        void*           ptr;
#ifdef WIN32
        __int32     u32;
        __int64     u64;
#else // linux
        __uint32_t  u32;
        __uint64_t  u64;
#endif // linux
    } minHeap_user_data;
    // //////////////////
    minHeap_user_data   user_data;

    //
    // �û������޸��������, Ҳ���ù����������
    // �ñ���ֻ���û���ģ���װ����ʹ��, ָ�򱣴汾min_event�Ķ���
    //
    void*               parent;

    min_event() {
        memset(this, 0, sizeof(struct min_event));
    }

    void Reset() {
        memset(this, 0, sizeof(struct min_event));
    }
};
typedef min_event*  min_event_ptr;

struct min_heap_t {
    struct min_event**  p;
    uint32_t            n, a;

    // //////////////////
    //
    // ��¼��ǰʱ��, ÿ�β����½ڵ��ʱ����t_lost = (t_now - t_begin)
    // t_Remain = t_duration-t_lost, t_RemainΪ�ڵ�ʣ�������ڳ�ʱʱ��,
    // �����ж���Щ�ڵ����쳬ʱ
    struct timeval t_now;
    // //////////////////
};

//
// ** Return true iff the tvp is related to uvp according to the relational
// * operator cmp.  Recognized values for cmp are  == ,  <= , <,  >= , and >.
//
#define evutil_timercmp(tvp, uvp, cmp)    ((tvp)->tv_sec == (uvp)->tv_sec) ? \
                                       ((tvp)->tv_usec cmp (uvp)->tv_usec) : \
                                       ((tvp)->tv_sec cmp (uvp)->tv_sec)

// -----------------------------------------------------------------------------
inline void              min_heap_ctor(min_heap_t* s);
inline void              min_heap_dtor(min_heap_t* s);
inline void              min_heap_elem_init(struct min_event* e);
inline int32_t           min_heap_elt_is_top(const struct min_event *e);
// inline int32_t        min_heap_elem_greater(struct event *a,
//                                             struct event *b);
inline int32_t           min_heap_empty(min_heap_t* s);
inline uint32_t          min_heap_size(min_heap_t* s);
inline struct min_event* min_heap_top(min_heap_t* s);
inline int32_t           min_heap_reserve(min_heap_t* s, uint32_t n);
inline int32_t           min_heap_push(min_heap_t* s, struct min_event* e);
inline struct min_event* min_heap_pop(min_heap_t* s);
inline int32_t           min_heap_erase(min_heap_t* s, struct min_event* e);
inline void              min_heap_shift_up_(min_heap_t* s, uint32_t hole_index,
        struct min_event* e);
inline void              min_heap_shift_down_(min_heap_t* s,
        uint32_t hole_index,
        struct min_event* e);
// -----------------------------------------------------------------------------

inline void              min_heap_ctor(min_heap_t* s) {
    s->p = 0;
    s->n = 0;
    s->a = 0;
}

inline void              min_heap_dtor(min_heap_t* s) {
    delete [](s->p);
}
inline void              min_heap_elem_init(struct min_event* e) {
    e->ev_timeout_pos.min_heap_idx = -1;
}

inline int32_t           min_heap_empty(min_heap_t* s) {
    return 0u == s->n;
}
inline uint32_t          min_heap_size(min_heap_t* s) {
    return s->n;
}
inline struct min_event* min_heap_top(min_heap_t* s) {
    return s->n ? *s->p : 0;
}

//
// �Ƚ��Ƿ�a >b
//
inline int32_t min_heap_elem_greater(min_heap_t* s, struct min_event *a,
                                     struct min_event *b) {
    // a
    // ���ŵ�ʱ�� t_lost = t_now - t_begin
    //
    long t_lost_a = (s->t_now.tv_sec - a->t_begin.tv_sec)*1000000 +
                    s->t_now.tv_usec -
                    a->t_begin.tv_usec;

    //
    // ʣ���ʱ�� t_remain = t_duration - t_lost
    //
    long t_remain_a = a->t_duration.tv_sec*1000000 +
                      a->t_duration.tv_usec -
                      t_lost_a;

    t_remain_a = t_remain_a >0 ? t_remain_a : 0;

    // b
    // ���ŵ�ʱ�� t_lost = t_now - t_begin
    //
    long t_lost_b = (s->t_now.tv_sec - b->t_begin.tv_sec)*1000000 +
                    s->t_now.tv_usec -
                    b->t_begin.tv_usec;

    //
    // ʣ���ʱ�� t_remain = t_duration - t_lost
    //
    long t_remain_b = b->t_duration.tv_sec*1000000 +
                      b->t_duration.tv_usec -
                      t_lost_b;

    t_remain_b = t_remain_b >0 ? t_remain_b : 0;

    struct timeval remain_a = {t_remain_a/1000000, t_remain_a%1000000};
    struct timeval remain_b = {t_remain_b/1000000, t_remain_b%1000000};

    return evutil_timercmp(&remain_a, &remain_b, >);
}

inline int32_t min_heap_erase(min_heap_t* s, struct min_event* e) {
    if ((-1) != e->ev_timeout_pos.min_heap_idx) {
        struct min_event* last = s->p[--s->n];
        uint32_t parent = (e->ev_timeout_pos.min_heap_idx - 1) / 2;

        //
        // we replace e with the last element in the heap.  We might need to
        // shift it upward if it is less than its parent, or downward if it is
        // greater than one or both its children. Since the children are known
        // to be less than the parent, it can't need to shift both up and
        // down.
        //

        if (e->ev_timeout_pos.min_heap_idx > 0 &&
                min_heap_elem_greater(s, s->p[parent], last))
            min_heap_shift_up_(s, e->ev_timeout_pos.min_heap_idx, last);
        else
            min_heap_shift_down_(s, e->ev_timeout_pos.min_heap_idx, last);
        e->ev_timeout_pos.min_heap_idx = -1;
        return 0;
    }
    return -1;
}

inline int32_t min_heap_push(min_heap_t* s, struct min_event* e) {
    if (min_heap_reserve(s, s->n + 1))
        return -1;

    //
    // ����head��tnow, ������������
    //
    struct timeval tm = {0};
    lite_gettimeofday(&tm, 0);
    s->t_now.tv_sec = tm.tv_sec;
    s->t_now.tv_usec = tm.tv_usec;
    // //////////////////

    //
    // ��ʼ���ڵ��t_begin
    //
    e->t_begin.tv_sec = tm.tv_sec;
    e->t_begin.tv_usec = tm.tv_usec;

    min_heap_shift_up_(s, s->n++, e);
    return 0;
}

inline struct min_event* min_heap_pop(min_heap_t* s) {
    if (s->n) {
        struct min_event* e = *s->p;
        min_heap_shift_down_(s, 0u, s->p[--s->n]);
        e->ev_timeout_pos.min_heap_idx = -1;
        return e;
    }
    return 0;
}

inline int32_t min_heap_elt_is_top(const struct min_event *e) {
    return e->ev_timeout_pos.min_heap_idx == 0;
}



inline int32_t min_heap_reserve(min_heap_t* s, uint32_t n) {
    if (s->a < n) {
        struct min_event** p;
        uint32_t a = s->a ? s->a * 2 : 8;
        if (a < n)
            a = n;

        p = new min_event_ptr[a];
        if (p == 0)
            return -1; // out of memory
        if (s->p) {
            memcpy((unsigned char*)p,
                   (unsigned char*)(s->p),
                   s->a*sizeof(min_event_ptr));
            delete []s->p;
        }

        s->p = p;
        s->a = a;
    }
    return 0;
}

inline void min_heap_shift_up_(min_heap_t* s, uint32_t hole_index,
                               struct min_event* e) {
    uint32_t parent = (hole_index - 1) / 2;
    while (hole_index && min_heap_elem_greater(s, s->p[parent], e)) {
        (s->p[hole_index] = s->p[parent])->ev_timeout_pos.min_heap_idx =
            hole_index;
        hole_index = parent;
        parent = (hole_index - 1) / 2;
    }
    (s->p[hole_index] = e)->ev_timeout_pos.min_heap_idx = hole_index;
}

inline void min_heap_shift_down_(min_heap_t* s, uint32_t hole_index,
                                 struct min_event* e) {
    uint32_t min_child = 2 * (hole_index + 1);
    while (min_child <= s->n) {
        min_child -= min_child == s->n ||
                     min_heap_elem_greater(s,
                                           s->p[min_child],
                                           s->p[min_child - 1]);

        if (!(min_heap_elem_greater(s, e, s->p[min_child])))
            break;
        (s->p[hole_index] = s->p[min_child])->ev_timeout_pos.min_heap_idx =
            hole_index;

        hole_index = min_child;
        min_child = 2 * (hole_index + 1);
    }
    min_heap_shift_up_(s, hole_index,  e);
}

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_MIN_HEAP_H_
