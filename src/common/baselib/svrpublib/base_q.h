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
// description: ������ʽ����ڵ�, ֧�ֶ��̵߳���
//
//             1:��֤FIFO(first in first out)
//             2:�����һ������, ȡ����һ������.
//               ȡ��������Ϊ��ʱȥ��ȡ����˶���,
//               �ڵ���Ŀ�Զ����û�ʹ�ù�ģ��ʱ��, ������پ�������.
//             3:�û��ṹֻ��Ҫ֧��*next��Ա����.
//             4:ֻ����ڵ����, �������ڴ����
//
template<typename TNode>
class CBaseQ {
public:
    bool AppendNodeAtTail(TNode* &node);
    bool GetNodeOnHead(TNode** node);       //  ��ʵȡ���ڵ�
    TNode* GetHeadNodePtr();                //  �������ֻȡһ���ڵ�Ĳο�,
    //  ������ʵȡ���ڵ�
    int32_t GetNodesCount();

    //
    // ��AppendNodeAtTail()ʱ����Get��Ϊ��, ��ص��û��趨�Ļص�����
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
// 1:��⽫Ҫ����Ľڵ��Ƿ��Ѿ�����, ���������ǿ��core down
// 2:_DEBUG����������Զ����, Ч�ʽϵ�
// 3:��_DEBUG״̬���Զ��ر�
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
//  pCallback:��Get��Ϊ�ղŴ����ص�
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
        // �ж�Get���Ƿ�Ϊ��, Ϊ����ص��û��趨�Ļص��ӿ�
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
// ��AppendNodeAtTail()ʱ����Get��Ϊ��, ��ص��û��趨�Ļص�����
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
