// base_q.h: interface for the CBaseQ class.
// wookin@tencent.com    2010/07/12
//
// /////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_BASE_Q_H_
#define COMMON_BASELIB_SVRPUBLIB_BASE_Q_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

//
// class:
// description: 单链表方式管理节点, 支持多线程调用
//
//             1:保证FIFO(first in first out)
//             2:插入端一个队列, 取出端一个队列.
//               取出端数据为空时去获取插入端队列,
//               节点数目略多于用户使用规模的时候, 会大大减少竞锁几率.
//             3:用户结构只需要支持*next成员变量.
//             4:只负责节点队列, 不负责内存管理
//
template<typename TNode>
class CBaseQ {
public:
    bool AppendNodeAtTail(TNode* &node);
    bool GetNodeOnHead(TNode** node);       //  真实取出节点
    TNode* GetHeadNodePtr();                //  这个函数只取一个节点的参考,
    //  并不真实取出节点
    int32_t GetNodesCount();

    //
    // 当AppendNodeAtTail()时候发现Get端为空, 则回调用户设定的回调函数
    //
    void SetNotifyCallback(Closure<void>* permanent_callback);

    void   Reset() {
        m_queue_get_head = m_queue_put_head = m_queue_put_tail = 0;
        m_num_in_get_queue_count = m_num_in_put_queue_count = 0;
    }

    CBaseQ() {
        m_queue_get_head = m_queue_put_head = m_queue_put_tail = 0;
        m_num_in_get_queue_count = m_num_in_put_queue_count = 0;
        m_notify_callback = NULL;
    }

    ~CBaseQ() {}
private:
    bool GetNodeOnHeadPtr(TNode** node, bool get_ref_only);

    Closure<void>*      m_notify_callback;

    // Get queue
    TNode*  volatile    m_queue_get_head;
    int32_t  volatile   m_num_in_get_queue_count;

    // Put queue
    TNode*  volatile    m_queue_put_head;
    TNode*  volatile    m_queue_put_tail;
    int32_t  volatile   m_num_in_put_queue_count;

    CXThreadMutex       m_put_mutex;
    CXThreadMutex       m_get_mutex;
};

//
// start:DEBUG_NODE_EXIST
// 1:检测将要加入的节点是否已经存在, 如果存在则强行core down
// 2:_DEBUG开启情况下自动监测, 效率较低
// 3:无_DEBUG状态在自动关闭
//
#if defined(_DEBUG) && defined(_DEBUG_NODE_EXIST)
#define DEBUG_NODE_EXIST(NodeType, ptrNodesHead, insert_node_8868)  \
                            NodeType* temp_head_8868 = ptrNodesHead;    \
                            while (temp_head_8868)                      \
                            {                                           \
                                if (temp_head_8868= = insert_node_8868) \
                                {                                                         \
                                 LOG(FATAL) << "***ERROR, Level 3***,"                    \
                                 " node:" << reinterpret_cast<void*>(insert_node_8868) << \
                                 " already in queue.";                                    \
                                }                                                         \
                                temp_head_8868 = temp_head_8868->next;                    \
                             }
#else
#define DEBUG_NODE_EXIST(NodeType, ptrNodesHead, insert_node_8868)
#endif //

#define _CHECK_NODE_STATE(ptr)                               \
    if (!ptr || ptr->node_state == NODE_STATE_IN_FREE_QUEUE) \
    {                                                        \
       LOG(ERROR) << "***ERROR, Level 3***, "                \
                     "invalid node or node:"                 \
                     "0x" << reinterpret_cast<void*>(ptr) << \
                     " already in free queue";               \
       return false;                                         \
    }

//
//  Class CBaseQ
//  pCallback:当Get端为空才触发回调
//
template<typename TNode>
bool CBaseQ<TNode>::AppendNodeAtTail(TNode* &node) {
    // _CHECK_NODE_STATE(ptrNode);
    // ptrNode->ucNodeState = NODE_STATE_IN_FREE_QUEUE;

    node->next = NULL;

    m_put_mutex.Lock();
    DEBUG_NODE_EXIST(TNode, m_queue_put_head, node);
    if (!m_queue_put_head) {
        m_queue_put_head = m_queue_put_tail = node;
    } else {
        m_queue_put_tail->next = node;
        m_queue_put_tail = m_queue_put_tail->next;
    }
    ++m_num_in_put_queue_count;
    node = NULL;

    m_put_mutex.UnLock();

    if (m_notify_callback) {
        //
        // 判断Get端是否为空, 为空则回调用户设定的回调接口
        //
        bool is_empty = false;
        m_get_mutex.Lock();
        is_empty = (m_queue_get_head == NULL);
        m_get_mutex.UnLock();

        if (is_empty)
            m_notify_callback->Run();
    }
    return true;
}

//
// 当AppendNodeAtTail()时候发现Get端为空, 则回调用户设定的回调函数
//
template<typename TNode>
void CBaseQ<TNode>::SetNotifyCallback(Closure<void>* permanent_callback) {
    m_notify_callback = permanent_callback;
}

template<typename TNode>
bool CBaseQ<TNode>::GetNodeOnHeadPtr(TNode** node, bool get_ref_only) {
    bool b = false;
    m_get_mutex.Lock();

    // Prepare nodes
    if (!m_queue_get_head) {
        m_put_mutex.Lock();
        m_queue_get_head = m_queue_put_head;
        m_queue_put_head = m_queue_put_tail = 0;
        m_num_in_get_queue_count = m_num_in_put_queue_count;
        m_num_in_put_queue_count = 0;
        m_put_mutex.UnLock();
    }

    if (m_queue_get_head) {
        *node = m_queue_get_head;
        if (!get_ref_only) {
            m_queue_get_head = m_queue_get_head->next;
            (*node)->next = 0;
            // (*pptrNode)->ucNodeState = NODE_STATE_IN_USE;
            m_num_in_get_queue_count--;
        }
        b = true;
    }

    m_get_mutex.UnLock();
    return b;
}

template<typename TNode>
bool CBaseQ<TNode>::GetNodeOnHead(TNode** node) {
    return GetNodeOnHeadPtr(node, false);
}

template<typename TNode>
TNode* CBaseQ<TNode>::GetHeadNodePtr() {
    TNode* ptr = 0;
    GetNodeOnHeadPtr(&ptr, true);
    return ptr;
}

template<typename TNode>
int32_t CBaseQ<TNode>::GetNodesCount() {
    m_get_mutex.Lock();
    m_put_mutex.Lock();
    int32_t num_count = m_num_in_get_queue_count + m_num_in_put_queue_count;
    m_put_mutex.UnLock();
    m_get_mutex.UnLock();
    return num_count;
}

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_BASE_Q_H_
