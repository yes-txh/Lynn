// sink_nodes_t.h: interface for the CSinkNodes_T class.
// wookin@tencent.com    2006/12/20
// ////////////////////////////////////////////////////////////////////
#ifndef COMMON_BASELIB_SVRPUBLIB_SINK_NODES_T_H_
#define COMMON_BASELIB_SVRPUBLIB_SINK_NODES_T_H_

#include "common/baselib/svrpublib/base_config.h"

#ifdef WIN32
#pragma   warning(disable:4127)
#endif // WIN32

_START_XFS_BASE_NAMESPACE_

//
//  保护节点, 防止应用层操作失误重复加入节点
//  1:应用节点里面需要加变量ucNodeState
//  加入到CLinkedListNodesMgr内部的节点全部状态为NODE_STATE_IN_FREE_QUEUE
//  在使用中的节点状态全部为NODE_STATE_IN_USE
//
enum ENUM_NODE_STATE {
    NODE_STATE_IN_FREE_QUEUE = 100,
    NODE_STATE_IN_USE = 101,
};

//
//  Dual linked list
//  Class CDualLinkListQueueMgr_T
//  外部管理内存
//
template<typename TNode> class CDualLinkListQueueMgr_T {
public:
    virtual ~CDualLinkListQueueMgr_T() {}
    CDualLinkListQueueMgr_T() {
        m_head = m_tail = 0;
        m_current_nodes_count = 0;
    }

    bool        AppendNodeAtTail(TNode* &node);
    bool        InsertNode(TNode* forward_node, TNode* insert_node);
    bool        GetNodeOnHead(TNode** node);

    //
    // Remove node from queue
    //
    bool        RemoveNode(TNode* node);

    // 用下面几个接口的时候需要小心，如使用m_head的过程中m_head可能被其它线程修改
    TNode*      GetHeadNodePtr() {
        return m_head;
    }
    TNode*      GetTailNodePtr() {
        return m_tail;
    }
    uint32_t    GetNodesCount()const {
        return m_current_nodes_count;
    }

    void    SetQueueEmpty() {
        if (m_head || m_tail) {
            LOG(ERROR) << "dual link list, "
                       "SetQueueEmpty, but have some valid nodes, "
                       "m_head = " << reinterpret_cast<void*>(m_head) <<
                       ", m_tail = " << reinterpret_cast<void*>(m_tail);
        }
        m_head = m_tail = 0;
        m_current_nodes_count = 0;
    }
private:
    TNode*      volatile    m_head;
    TNode*      volatile    m_tail;
    uint32_t    volatile    m_current_nodes_count;
    CXThreadMutex           m_mutex;
};

//
//  Single linked list
//  Class CLinkListQueueMgr_T
//  外部管理内存
//
template<typename TNode> class CLinkListQueueMgr_T {
public:
    CLinkListQueueMgr_T();
    virtual ~CLinkListQueueMgr_T();

    bool    AppendNodeAtTail(TNode* &node) {
        return m_queue.AppendNodeAtTail(node);
    }

    bool    GetNodeOnHead(TNode** node) {
        return m_queue.GetNodeOnHead(node);
    }

    TNode*  GetHeadNodePtr() {
        return m_queue.GetHeadNodePtr();
    }

    uint32_t    GetNodesCount() {
        return m_queue.GetNodesCount();
    }
private:
    CBaseQ<TNode>   m_queue;
};

//
//  Linked list nodes manage
//  Class CLinkedListNodesMgr
//
//  本类自己管理内存
//  所有节点放于一个链表上，从尾部添加，头部获取
//
template<typename TNode> class CLinkedListNodesMgr {
public:
    CLinkedListNodesMgr();
    virtual ~CLinkedListNodesMgr();

    bool    Init(uint32_t nodes);
    void    Uninit();
    bool    IsQueueEmpty();
    bool    ReconfigNodes(uint32_t nodes);
    bool    AppendNodeAtTail(TNode* &node) {
        return m_queue.AppendNodeAtTail(node);
    }

    bool    GetNodeOnHead(TNode** node) {
        return m_queue.GetNodeOnHead(node);
    }

    uint32_t    GetMaxNodesCount()const {
        return m_nodes_count;
    }

    void        Reset();

    // 支持m64
    uint64_t    GetNodeIndex(TNode* node);
    TNode*      GetNodePtrByIndex(uint32_t index);

    // 校验指针是否合法, 指针范围及具体取值
    bool    IsValidNodePtr(const TNode* node);
    int32_t  GetCurrValidNodes() {
        return m_queue.GetNodesCount();
    }
private:
    CBaseQ<TNode>           m_queue;
    TNode*                  m_nodes_buff;
    uint32_t    volatile    m_nodes_count;
};

//
// 按降序排序
//
template<typename TNode, typename TValType>
void QuickSort_DESC(int32_t begin, int32_t end, TNode* nodes_array);

#include "common/baselib/svrpublib/sink_nodes_t.inl"

//
//  智能节点管理, 尽量同一个线程使用自己的节点, 则不会造成竞锁
//
template<typename TNode, uint32_t T_nodes_count = 0 >
class CSmartNodes {
public:
    CSmartNodes() {
#if (defined(_DEBUG))
        m_put_nodes_count = m_get_nodes_count = 0;
#endif // _DEBUG
        uint32_t uNodesCount = T_nodes_count;
        if (uNodesCount>0) {
            if (!m_nodes_list.Init(T_nodes_count)) {
                VLOG(3) << "CSmartNodes, init fail, "
                          "sizeof(TNode) = " << static_cast<uint32_t>(sizeof(TNode)) <<
                          " uNodesCount = " << T_nodes_count;
            } else {
#if (defined(_DEBUG))
                VLOG(3) << "new " << T_nodes_count << "%u nodes of type:" <<
                          typeid(TNode).name();
#endif // _DEBUG
            }
        }
    }

    //
    //  如果模板参数T_uNodesCount为0,
    //  则需要显示调用InitNodes()进行初始化
    //
    bool InitNodes(uint32_t nodes) {
#if (defined(_DEBUG) || defined(_DEBUG_COUNT))
        if (nodes) {
            VLOG(3) << "new " << nodes << " nodes of type:" <<
                      typeid(TNode).name();
        }
#endif // _DEBUG
        return m_nodes_list.Init(nodes);
    }

    virtual ~CSmartNodes() {
#if (defined(_DEBUG) || defined(_DEBUG_COUNT))
        VLOG(3) << "delete nodes of type:" << typeid(TNode).name();
#endif //
        m_nodes_list.Uninit();;
    }

    TNode* GetNode() {
        TNode* ptr = 0;
        m_nodes_list.GetNodeOnHead(&ptr);
#if (defined(_DEBUG) || defined(_DEBUG_COUNT))
        if (ptr) {
            m_num_mutex.Lock();
            m_get_nodes_count++;
            m_num_mutex.UnLock();
            VLOG(3) << "GetNode(name:" << typeid(ptr).name() << "), get:" <<
                      m_get_nodes_count << ", put:" << m_put_nodes_count << ", "
                      "outside: get-put = " << m_get_nodes_count - m_put_nodes_count;
        } else {
            VLOG(3) << "get node of type:" << typeid(ptr).name() << " fail, ptr = 0x00";
            DebugNodesInfo(true);
        }
#endif // _DEBUG
        return ptr;
    }

    void PutNode(TNode* &node) {
#if (defined(_DEBUG) || defined(_DEBUG_COUNT))
        m_num_mutex.Lock();
        if (node)
            m_put_nodes_count++;
        m_num_mutex.UnLock();
        DebugNodesInfo(false);
#endif // _DEBUG
        node->next = 0;
        m_nodes_list.AppendNodeAtTail(node);
    }

    void DebugNodesInfo(bool get) const {
#if (defined(_DEBUG) || defined(_DEBUG_COUNT))
        const char* type = 0;
        if (get)
            type = "*GET*";
        else
            type = "*PUT*";

        const uint32_t nodes_count = m_nodes_list.GetMaxNodesCount();
        VLOG(3) << type << ": type:" << typeid(TNode).name() <<
                  ", total nodes:" << nodes_count << ", times count, "
                  "get:" << m_get_nodes_count << ", put:" << m_put_nodes_count <<
                  ", out side:put-get:" << m_get_nodes_count - m_put_nodes_count;
#endif // _DEBUG
    }

    uint64_t GetNodeIndex(TNode* node) {
        return m_nodes_list.GetNodeIndex(node);
    }

    TNode* GetNodePtrByIndex(uint32_t index) {
        return m_nodes_list.GetNodePtrByIndex(index);
    }

    bool ReconfigNodes(uint32_t nodes) {
        return m_nodes_list.ReconfigNodes(nodes);
    }

    int32_t GetCurrFreeNodes() {
        return m_nodes_list.GetCurrValidNodes();
    }

private:
    CLinkedListNodesMgr<TNode>  m_nodes_list;
    CXThreadMutex               m_num_mutex;

#if (defined(_DEBUG) || defined(_DEBUG_COUNT))
    int32_t volatile            m_put_nodes_count;
    int32_t volatile            m_get_nodes_count;
#endif // _DEBUG
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_SINK_NODES_T_H_
