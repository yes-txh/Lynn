// data_queue.h
// variant data node
// wookin@tencent.com
// 2008.05.26
//
// 修改:1:增加队尾, 支持先进先出
//
//      2:除消息以外还支持按socket pair方式触发(Init()设定需要通知的socket
//          handle获得函数)
//
//      3:**Socket pair消息通知时, 当队列为空可以调用用户回调接口GetQueue(...,
//          pCallbackWhenEmpty)
// 2010-07-07 wookin
//

#ifndef COMMON_BASELIB_SVRPUBLIB_DATA_QUEUE_H_
#define COMMON_BASELIB_SVRPUBLIB_DATA_QUEUE_H_

//
// Closure
//
#include "common/base/closure.h"
#include "common/baselib/svrpublib/base_config.h"

#ifdef WIN32
#pragma   warning(disable:4127)
#endif // WIN32

_START_XFS_BASE_NAMESPACE_

template<typename T>
class IQueuePut {
public:
    // 添加到这个队列之后, pNode指针就被置空了, 这样保证pNode只有唯一一个copy
    virtual void AddSingleNode(T* &node) = 0;

    IQueuePut() {}
    virtual ~IQueuePut() {}
};

template<typename T>
class IQueueGet {
public:
    //
    // max_queue = 0, 一次获取所有的节点
    //
    virtual bool GetQueue(T** node, uint32_t max_queue = 3,
                          Closure<void>* callback_when_empty = 0) = 0;

    IQueueGet() {}
    virtual ~IQueueGet() {}
};

//
// class CVDataQueue_T
// 简单数据Queue
// 适合多线程
// T->next;
//
template <typename T>
class CVDataQueue_T:public IQueuePut<T>, public IQueueGet<T> {
public:
    virtual void AddSingleNode(T* &node) {
        if (!node)
            return;

        bool post = false;
        m_mutex.Lock();
        ++m_current_nodes;
        node->next = NULL;

        post = m_need_post_event || (m_head == NULL);
        if(m_need_post_event)
            m_need_post_event = false;

        if (!m_tail) {
            m_head = m_tail = node;
        }
        else {
            m_tail->next = node;
            m_tail = m_tail->next;
        }
        node = NULL;
        m_mutex.UnLock();

        // ? Set event
        if (post)
            PostEvent();
    }

    //
    // max_queue = 0, 一次获取所有的节点
    // callback_when_empty, 当队列为空则回调这个函数
    //
    virtual bool GetQueue(T** node, uint32_t max_queue = 3,
                          Closure<void>* callback_when_empty = NULL) {
        if (!m_head)    // try get notify message
            WaitForEvent(m_timeout);

        if (!m_head)
            return false;

        // 支持多线程调用
        // have nodes
        CXThreadAutoLock autoLock(&m_mutex);

        if (!m_head)
            return false;   // why ?
                            // 有可能m_pHead在进入m_Mutex.Lock()之前的
                            // 一瞬间刚好被别的线程抢走

        if (max_queue == 0) {
            // 一次获取所有的节点
            *node = m_head;
            m_head = m_tail = NULL;
            m_current_nodes = 0;
            if (!m_sem_event && callback_when_empty)
                callback_when_empty->Run();
        } else {
            uint32_t count = 0;
            T* temp_head = m_head;
            T* temp_tail = m_head;
            while (m_head) { // Get nodes(less than max Queue)
                // Seek to next
                temp_tail = m_head;
                m_head = m_head->next;

                // Count
                ++count;
                if (count >= max_queue)
                    break;
            }

            if (temp_tail->next == 0) //  ? 最后一个节点
                m_head = m_tail = NULL;
            else
                temp_tail->next = NULL;

             // 如果队列还有数据,则加入节点数据的时候需要再次触发通知
            if(m_head)
                m_need_post_event = true;

            // Get queue
            *node = temp_head;
            m_current_nodes -= count;
            if (!m_sem_event && m_current_nodes == 0 && callback_when_empty){
                // 这种情况下队列必须为空
                CHECK(m_head == NULL);
                callback_when_empty->Run();
            }
        }
        return ((*node) != NULL);
    }

    CVDataQueue_T() {
        m_head = m_tail = NULL;
        m_sem_event = NULL;
        m_callback_get_notify_sock = NULL;
        m_current_nodes = 0;

        m_timeout = 20; // 默认设置为20 millisecs
        m_need_post_event = false;
    }

    virtual ~CVDataQueue_T() {
        // release all in use nodes
        T* ptmp = NULL;
        while (m_head) {
            ptmp = m_head;

            //
            // _memp_RELEASE_NODE(ptmp);
            // 默认是使用内存池new的节点
            //
            mempool_DELETE(ptmp);
            m_head = m_head->next;
        }
        m_tail = NULL;
        m_current_nodes = 0;
        m_need_post_event = false;

        Sem_CloseHandle(m_sem_event);
    }

    //
    // 如果需要在有数据的时候通知Get端, 则需要调用Init()初始化 notify handle
    // notify_handle:
    //               Sem_Event, 信号量
    //               tcp socket pair, tcp_connection连接对,
    //               如果Get端提交给Epoll则推荐使用这个
    //
    // callback_get_notify_sock ? SocketPair: Sem_CreateEvent
    //
    //
    void Init(Closure<void, SOCKET**>* callback_get_notify_sock = NULL) {
        // notify event
        if (callback_get_notify_sock)
            m_callback_get_notify_sock = callback_get_notify_sock;
        else {
            m_sem_event = Sem_CreateEvent(NULL, false, false, NULL);
        }
    }

    // post一个空消息
    // 如果另一端某个线程正在等待数据,可以提前结束等待
    void NotifyQuitMessage() {
        PostEvent();
    }

    void Uninit() {
        Sem_CloseHandle(m_sem_event);
    }

    int32_t GetNodesCountInQueue() {
        return m_current_nodes;
    }

    // set timeout, millisecs
    void SetTimeout(int32_t timeout) {
        m_timeout = timeout;
    }

private:
    void PostEvent() {
        CXThreadAutoLock auto_post_lock(&m_post_mutex);

        if (m_sem_event) {
            Sem_SetEvent(m_sem_event);
        } else {
            // notify socket pair message
            SOCKET* sock = NULL;
            m_callback_get_notify_sock->Run(&sock);
            if (*sock != INVALID_SOCKET) {
                int32_t len = 0;
                while((len = SendDat(*sock, "x", 1, 0)) != 1){
                    if(len == 0)
                        continue; // retry
                    if(len < 0){  // error
                        // Failed to send message, close socket pair
                        int32_t err = GetLastSocketError();
                        if(err == EAGAIN || err == POSIX_EWOULDBLOCK)
                            continue;

                        // 出错
                        LOG(ERROR) << "post socket pair notify message fail. err:" 
                                   << err << " ," << strerror(err);

                        // 防止另一个线程先重建socket pair
                        SOCKET fd = *sock;
                        *sock = INVALID_SOCKET;
                        CloseSocket(fd);                        
                        break;
                    }
                }

                if (len != 1){
                    m_mutex.Lock();
                    m_need_post_event = true;
                    m_mutex.UnLock();
                }
            }else{
                LOG(FATAL) << "get notify socket handle fail.";
            }
        }
    }

    void WaitForEvent(uint32_t timeout_millisecs) {
        if (m_sem_event) {
            Sem_WaitForSingleObject(m_sem_event, timeout_millisecs);
        }
    }

    T* volatile         m_head;
    T* volatile         m_tail;
    CXThreadMutex       m_mutex;
    int32_t  volatile   m_current_nodes;
    int32_t             m_timeout; // millisecs,设置每次等待输入端加入数据的最长等待时间
    bool                m_need_post_event;    // post event

    //
    // Sem event or Socket pair
    // m_hSemEvent:信号量触发
    // m_pcbGetNotifySock:socket消息触发, 这种模式下本类中WaitForEvent()无效,
    //                          用于另外一个线程使用epoll, select探测有数据到达
    //
    THANDLE                      m_sem_event;
    Closure<void, SOCKET**>*     m_callback_get_notify_sock;

    // mutex for post event
    CXThreadMutex       m_post_mutex;
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_DATA_QUEUE_H_
