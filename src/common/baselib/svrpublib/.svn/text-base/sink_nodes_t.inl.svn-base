//  sink_nodes_t.inl
//  Class CLinkListQueueMgr_T
//
template<typename TNode>
CLinkListQueueMgr_T<TNode>::CLinkListQueueMgr_T() {
}

template<typename TNode>
CLinkListQueueMgr_T<TNode>::~CLinkListQueueMgr_T() {
}

//
//  Dual list nodes manage
//  Class CDualLinkListQueueMgr_T
//
template<typename TNode>
bool CDualLinkListQueueMgr_T<TNode>::AppendNodeAtTail(TNode* &node) {
    CXThreadAutoLock autolock(&m_mutex);
    DEBUG_NODE_EXIST(TNode, m_head, node);
    bool b = false;

    // 避免部分重复加入
    if (node && !(node == m_head || node == m_tail)) {
        //  检查节点状态
        if (node->node_state == NODE_STATE_IN_FREE_QUEUE) {
            LOG(ERROR) << "*****ERROR, Level 5*****, "
                       "append node at dual tail fail, "
                       "the node:0x" << reinterpret_cast<void*>(node)
                       << " already in free queue.";
            return false;   //  Already in free queue
        }

        node->next = 0;
        node->pre = 0;
        node->node_state = NODE_STATE_IN_FREE_QUEUE;
        if (!m_head) {
            m_head = m_tail = node;
            m_current_nodes_count = 1;
        } else {
            m_tail->next = node;
            node->pre = m_tail;
            m_tail = m_tail->next;
            m_current_nodes_count++;
        }

#ifdef _DEBUG
        // Debug only
        if (m_current_nodes_count == 0) {
            LOG(FATAL) << "m_current_nodes_count = 0";
        }
#endif // _DEBUG

        node = 0;
        b = true;
    }
    return b;
}

template<typename TNode>
bool CDualLinkListQueueMgr_T<TNode>::InsertNode(TNode* forward_node, TNode* insert_node) {
    if (!insert_node) {
        return false;
    }
    CXThreadAutoLock autolock(&m_mutex);
    DEBUG_NODE_EXIST(TNode, m_head, insert_node);

    insert_node->pre  = NULL;
    insert_node->next = NULL;
    insert_node->node_state = NODE_STATE_IN_FREE_QUEUE;

    if (!forward_node) { // 插入到队头:note:这个时候队列可能为空
        if (!m_head) { // 队列为空
            CHECK_EQ(m_current_nodes_count, 0u);
            m_head = m_tail = insert_node;
            m_current_nodes_count = 1;
        } else { // 队头也可能有一个节点，需要修改队列尾节点
            if (1 == m_current_nodes_count ) { // 插入节点成为队头,需要修改队列尾节点
                m_tail->pre          = insert_node;
                insert_node->next    = m_tail;
                m_head               = insert_node;
            } else {
                m_head->pre          = insert_node;
                insert_node->next    = m_head;
                m_head               = insert_node;
            }
            ++m_current_nodes_count;
        }
    } else { // 这个时候队列中可能只有一个节点，队头队尾一样 --！
        if (1 == m_current_nodes_count) { // 插入节点成为新的队尾
            forward_node->next  = insert_node;
            insert_node->pre    = forward_node;
            m_tail              = insert_node;
        } else { // 插入节点可能成为新的队尾
            insert_node->next = forward_node->next; // 挂接后一节点
            forward_node->next= insert_node;        // 挂接插入节点
            insert_node->pre  = forward_node;       // 挂接前一节点
            if (!insert_node->next) {
                m_tail = insert_node;               // 成为新的队尾
            } else {
                insert_node->next->pre = insert_node; // 挂接新的前置节点
            }
        }
        ++m_current_nodes_count;
    }
    return true;
}

template<typename TNode>
bool CDualLinkListQueueMgr_T<TNode>::GetNodeOnHead(TNode** node) {
    CXThreadAutoLock autolock(&m_mutex);

    bool b = false;
    if (!node)
        return b;

    *node = 0;
    if (m_head) {
        *node = m_head;
        m_head = m_head->next;
        if (m_head)
            m_head->pre = 0;

        // ? Last one
        if (!m_head)
            m_tail = NULL;

        m_current_nodes_count--;
        (*node)->next = 0;
        (*node)->pre = 0;
        (*node)->node_state = NODE_STATE_IN_USE;
        b = true;

        // Debug only
#ifdef _DEBUG
        if (m_current_nodes_count == 0 && (m_head || m_tail)) {
            LOG(FATAL) << "m_current_nodes_count = 0, m_head = 0x"
                       << reinterpret_cast<void*>(m_head)
                       << ", m_tail = 0x" << reinterpret_cast<void*>(m_tail);
        }
#endif // _DEBUG
    }
    return b;
}

//
// Remove node from dual link list queue
//
template<typename TNode>
bool CDualLinkListQueueMgr_T<TNode>::RemoveNode(TNode* node) {
    CXThreadAutoLock autolock(&m_mutex);

    bool b = false;
    if (!node)
        return b;
    if (node->node_state != NODE_STATE_IN_FREE_QUEUE) {
        LOG(ERROR) << "***ERROR, Level 3***, "
                   "RemoveNode from dual link list fail, "
                   "the node not in queue:0x" << reinterpret_cast<void*>(node);
        return b;
    }

    // Debug only
    if (m_current_nodes_count == 0) {
        LOG(FATAL) << "m_current_nodes_count = 0";
    }

    // Remove node from dual link list queue
    if (node) {
        TNode* old_pre = node->pre;
        TNode* old_next = node->next;
        if (old_pre)
            old_pre->next = old_next;
        else
            m_head = old_next;

        if (old_next)
            old_next->pre = old_pre;
        else
            m_tail = old_pre;
        node->next = 0;
        node->pre = 0;
        m_current_nodes_count--;
        node->node_state = NODE_STATE_IN_USE;
        b = true;
    }
    return b;
}



//
//  Linked list nodes manage
//  Class CLinkedListNodesMgr
//
template<typename TNode>
CLinkedListNodesMgr<TNode>::CLinkedListNodesMgr() {
    m_nodes_buff = 0;
    m_nodes_count = 0;
}

template<typename TNode>
CLinkedListNodesMgr<TNode>::~CLinkedListNodesMgr() {
}

template<typename TNode>
bool CLinkedListNodesMgr<TNode>::Init(uint32_t nodes) {
    bool b = false;
    m_nodes_count = nodes;
    if (m_nodes_count && !m_nodes_buff) {
        m_nodes_buff = new TNode[m_nodes_count];
        if (m_nodes_buff) {
            //  让用户在TNode内自己实现构造函数初始化数据
            for (uint32_t u = 0; u < m_nodes_count; u++) {
                TNode* node = &m_nodes_buff[u];
                AppendNodeAtTail(node);
            }
            b = true;
        }
    } else {
        if (m_nodes_count == 0) {
            LOG(ERROR) << "LinkedListNodesMgr Init fail, m_nodes_count = 0";
        }
        if (m_nodes_buff) {
            LOG(ERROR) << "m_nodes_buff = 0x" << reinterpret_cast<void*>(m_nodes_buff) <<
                       " already exist";
        }
    }
    return b;
}

template<typename TNode>
bool CLinkedListNodesMgr<TNode>::IsValidNodePtr(const TNode* node) {
    bool b = false;
    if (node && ((unsigned char*)node) >= ((unsigned char*)m_nodes_buff)) {
        b = (((unsigned char*)node) -
             ((unsigned char*)m_nodes_buff)) % sizeof(TNode) == 0 ?
            true :
            false;
    }
    return b;
}

template<typename TNode>
void CLinkedListNodesMgr<TNode>::Uninit() {
    delete []m_nodes_buff;
    m_nodes_buff = 0;

    m_queue.Reset();
    m_nodes_count = 0;
}

template<typename TNode>
bool CLinkedListNodesMgr<TNode>::ReconfigNodes(uint32_t nodes) {
    Uninit();
    return Init(nodes);
}

template<typename TNode>
void    CLinkedListNodesMgr<TNode>::Reset() {
    if (m_nodes_count) {
        memset(m_nodes_buff, 0, sizeof(TNode)*m_nodes_count);
        for (uint32_t u = 0; u < m_nodes_count; u++) {
            TNode* node = &m_nodes_buff[u];
            AppendNodeAtTail(node);
        }
    }
}

template<typename TNode>
uint64_t    CLinkedListNodesMgr<TNode>::GetNodeIndex(TNode* node) {
    uint64_t index = 0;
    uint64_t val = (unsigned char*)node-(unsigned char*)m_nodes_buff;
    if (node &&
            (unsigned char*)node >= (unsigned char*)m_nodes_buff &&
            val%sizeof(TNode) == 0) {
        index = val/sizeof(TNode);
    } else {
        LOG(ERROR) << "*** ERROR ***, GetNodeIndex, invalid input node.";
    }
    return index;
}

template<typename TNode>
TNode*  CLinkedListNodesMgr<TNode>::GetNodePtrByIndex(uint32_t index) {
    TNode* ptr = 0;
    if (index < m_nodes_count && m_nodes_buff) {
        ptr = &m_nodes_buff[index];
    }
    return ptr;
}

template<typename TNode>
bool CLinkedListNodesMgr<TNode>::IsQueueEmpty() {
    return (m_queue.GetNodesCount()>0) ? false:true;
}

template<typename TNode, typename TValType>
inline void QuickSort_DESC(int32_t begin, int32_t end, TNode* node_array) {
    if (begin >= end)
        return;

    if (begin + 1 == end) {
        if (node_array[begin].GetNodeVal()< node_array[end].GetNodeVal()) {
            TNode cNode = node_array[begin];
            node_array[begin] = node_array[end];
            node_array[end] = cNode;
        }
        return;
    }

    int32_t middle = (begin + end)/2;
    TValType udwMidValue = node_array[middle].GetNodeVal();
    int32_t m = begin;
    int32_t n = end;

    while (begin < end) {
        while (begin < end && node_array[begin].GetNodeVal() > udwMidValue)
            begin++;
        while (begin < end && node_array[end].GetNodeVal() < udwMidValue)
            end--;

        if (begin < end) {
            TNode cpTmp = node_array[begin];
            node_array[begin] = node_array[end];
            node_array[end] = cpTmp;

            begin++;
            if (begin < end)
                end--;
        }
    }

    if (node_array[begin].GetNodeVal() > udwMidValue)
        begin++;

    if (m < begin)
        QuickSort_DESC<TNode, TValType>(m, begin-1, node_array);
    if (n > end)
        QuickSort_DESC<TNode, TValType>(end, n, node_array);
}

//
//  struct kka
//  {
//      char    sz[10];
//        uint32_t c;
//        inline    uint32_t GetNodeVal(){return c;}
//  };
//  kka*  array = new kka[nMax];
//  for (int32_t i = 0;i<nMax;i++)
//        array[i].c = rand();
//  QuickSort_ASC<kka, uint32_t>(0, nMax-1, array);
//
//
