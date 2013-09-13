// interface_longconn.h  :interface for Long connection object
// wookin@tencent.com
// ////////////////////////////////////////////////////////////////////
//
// update list
// ___________________________
// data        by     detail
// 2010-02-17  lonely 支持一条链路两端设置包类型(请求还是响应or默认)
// 2010-02-21  lonely 每个任务组可以设置超时时间
// 2010-02-24  lonely 一个任务组中M个请求,如果M个下游都是同一种类型的，
// 可以指定当其中的N(N<=M)个返回后就回调或者再等待T(N)*ratio时间
// 后如果还有没有返回响应的也回调
// 一个任务组中M个请求，如果M个下游不是都是同一种类型的，
// 可以指定任何一个请求不是必须要等待响应才可以回调的 ；二次回调不需要
// 2011-05-24  lonely  长连接不再主动去connect关闭的链接，否则会导致应用层阻塞
// 2011-05-25  lonely  增加链接关闭的回调接口

#ifndef COMMON_BASELIB_SVRPUBLIB_INTERFACE_LONGCONN_H_
#define COMMON_BASELIB_SVRPUBLIB_INTERFACE_LONGCONN_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

// ////////////////////////
// Public struct
// ////////////////////////

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

//
// struct:
// description: Long connection session handle
//
struct LongConnHandle {
    // 指向LongConnSession
    void*           handle;
    uint32_t        serial_num;

    bool operator == (const LongConnHandle &other) const {
        return (handle == other.handle && serial_num == other.serial_num) ?
               true :
               false;
    }

    bool operator != (const LongConnHandle &other) const {
        return (handle != other.handle || serial_num != other.serial_num) ?
               true :
               false;
    }

    bool operator>(const LongConnHandle &other) const {
        return (handle > other.handle) ? true:false;
    }

    bool operator<(const LongConnHandle &other) const {
        return (handle < other.handle) ? true:false;
    }
};

//
// union:
// description: LongConnUserData
//
union LongConnUserData {
    void*                   ptr;
    uint32_t                u32;

#ifdef WIN32
    unsigned __int64   u64;
#else // linux
    __uint64_t         u64;
#endif
    LongConnHandle          h64_handle;
};



//
// struct:      LTask
// description: task & TasksGroup
//
struct LTask {
    LongConnHandle  long_conn_session;  // Long connection session
    unsigned char*  data;               // 用户待发送数据缓冲区
    uint32_t        valid_data_len;     // 有效数据长度
    unsigned char   need_recv_response; // 设置是否需要接收服务器回应数据
    unsigned char   is_need_finish;     // 设置回调是否必须等待该子任务完成

    unsigned char   is_pure_data;       // data指向的数据没有包头,
    // 纯数据:1 纯数据;0 已经有包头

    uint32_t        seq_num;            // is_pure_data = 1时设置才有效
    uint16_t        service_type;       // is_pure_data = 1时设置才有效
    uint16_t        pri_key;            // is_pure_data = 1时设置才有效

    //
    // 下面为返回状态, 调用者设定无效
    // 1:ok, happen
    // 0:nothing
    //
    unsigned char   _is_verify_pack_fail:1;     // ? 校验发送的请求包
    //   是否有效 :(
    unsigned char   _is_send_ok:1;              // ? 发送完毕
    unsigned char   _is_receive_ok:1;           // ? 接收完成
    unsigned char   _timeout_event:1;           // ? 有time out事件发生
    //
    unsigned char   _is_downstream_busy:1;      // ? downstream busy
    //   (or downstream
    //    out of nodes)

    unsigned char   _is_sock_already_closed:1;  // ? socket fd是否已经关闭,
    // 对于SOCK_DIR_ACCEPT_IN
    // 类型有用,可能fd已经被迫关闭

    void*           _node;                      // 数据收发缓冲区和内部节点管理
    // (LongConnNode)
    //
    unsigned char*  _received_data;             // 指向_pNode的数据区
    uint32_t        _received_data_len;         // _pNode的数据区的有效长度

    void SetSendData(unsigned char* dat, uint32_t len, uint32_t seq,
                     uint16_t service, uint16_t key) {
        SetSendData(dat, len);
        is_pure_data = 1;
        seq_num = seq;
        service_type = service;
        pri_key = key;
    }

    void SetSendData(unsigned char* dat, uint32_t len) {
        data = dat;
        valid_data_len = len;
        is_pure_data = 0;
    }

    void SetConnSession(LongConnHandle session_handle) {
        long_conn_session = session_handle;
    }

    void SetNeedResponse(unsigned char need_response) {
        need_recv_response = need_response;
    }

    void SetNeedFinish(unsigned char need_finish) {
        is_need_finish = need_finish;
    }

    LTask() {
        data = 0;
        _node = 0;

        long_conn_session.handle = 0;
        long_conn_session.serial_num = 0;
        valid_data_len = 0;
        need_recv_response = 0;
        is_need_finish = 1;

        // 替用户打包的相关变量
        is_pure_data = 0;
        seq_num = 0;
        service_type = 0;
        pri_key = 0;

        ResetState();
    }

    void ResetState() {
        _is_verify_pack_fail = _is_send_ok = _is_receive_ok = _timeout_event =
                _is_downstream_busy = _is_sock_already_closed = 0;

        _received_data = 0;
        _received_data_len = 0;
    }

    // 1:ok;0:fail
    int32_t IsTaskFinished() {
        int32_t state = 0;

        // 需要回应的任务
        if (need_recv_response && _is_send_ok && _is_receive_ok)
            state = 1;

        // 不需要response 的任务
        else if (!need_recv_response && _is_send_ok)
            state = 1;

        return state;
    }
};

//
// description: 缺省的任务组超时时间
//
const uint32_t kDefaultTasksGroupTimeoutVal =  1000; // ms

//
// description: 任务组回调策略
//
enum ENUM_TASKSGROUP_CALLBACK_STRATEGY {
    // CallBack Strategy 1: 任务组中所有的子任务都完成后回调
    // note：缺省
    STRATEGY_ALL_TASKS_MUST_FINISH = 0,

    // CallBack Strategy 2: 任务组中N个子任务中有M个完成，等待T(M)
    // *RATIO 时间后仍然有未完成的子任务也回调
    // note: 适用于N个子任务的下游都是同一种类型
    STRATEGY_N_TASKS_M_FINISH = 1,

    // CallBack Strategy 3: 任务组中每个子任务设置不是必须需要等待响应返回
    // 如N个子任务中M个设置了不是必须需要等待响应，当必须需要等待的N-M个都
    // 完成后，无论这个时候其它M个是否完成，都可以回调了
    // note: 适用于子任务的下游不是同一种类型，并发时可以提高性能，同时可以
    // 丢弃非必须下游数据进一步降低响应时间
    STRATEGY_SOME_TASK_NOT_MUST_FINISH = 2,
};

//
// struct: StrategyInfo
// description: 回调策略数据
//
struct StrategyInfo {
    ENUM_TASKSGROUP_CALLBACK_STRATEGY strategy_type;

    union strategy_data {
        struct {
            uint32_t need_net_finished_count;
            uint32_t last_finished_task_id;
        };
        int32_t need_finished_taskid_sum;
    } data;

    StrategyInfo() {
        strategy_type = STRATEGY_ALL_TASKS_MUST_FINISH;
    }
};

//
// struct: CallBackStrategyConfig
// description: 回调策略配置项
//
struct CallBackStrategyConfig {
    uint32_t time_cost_limit; // STRATEGY_N_TASKS_M_FINISH，超过此耗时
    float    ratio_by_last;   // 耗时是第M个的(1+ratio)*T(M)部分完成也回调

    CallBackStrategyConfig() {
        time_cost_limit = 0;    // 缺省时达到N-Mfinish立即回调
        ratio_by_last   = 0.0f;
    }
};

enum ENUM_TASKSGROUP_CALLBACK_STATUS {
    STATUS_ALL_FINISH     = 0, // 全部子任务都完成了
    STATUS_PARTIAL_FINISH = 1, // 部分子任务完成
    STATUS_REMAIN_FINISH  = 2, // 部分子任务完成回调后剩余的
    // 子任务也都完成后的回调
};
//
// struct:      LTasksGroup
// description: 任务组描述
//
template <uint32_t T_max_tasks>
struct LTasksGroup_T {
    uint32_t            m_valid_tasks;         // 前面连续n个tasks有效
    LongConnUserData    m_user_data1;          // user data 1
    LongConnUserData    m_user_data2;          // user data 2
    LongConnUserData    m_user_data3;          // user data 3
    LongConnUserData    m_user_data4;          // user data 4
    LongConnUserData    m_user_data5;          // user data 5
    uint32_t            m_timeout_val;         // 超时时间,ms
    StrategyInfo        m_strategy_info;       // task group strategy

    LTask               m_tasks[T_max_tasks];  // 所有子任务


    // //////////////////////////////
    // 下面为返回状态, 调用者设定无效
    // //////////////////////////////
    uint32_t            __net_finished_count;   // 当某个任务不再需要进一步
    // 操作网络的时候则
    // 认为net finished,
    // 比如:1:网络错误
    //      2:只发送的任务发送完毕,
    //      3:接收数据完毕(需要接收
    //        回应的任务)

    uint32_t            __task_finished_count;  // 统计逻辑任务完成的情况
    // 1:只需要发送的任务发送完毕
    // 2:需要接收请求的任务接收完毕

    uint32_t            __valid_connections;    // 统计本次发送列表中有效的
    // 连接个数,
    // 避免后台某些服务未启动

    uint32_t            __need_callback_count;  // 任务组中只要有一个任务需要rsp，就需要call back

    uint32_t            __group_callback_status; // 回调时的状态:ENUM_TASKSGROUP_CALLBACK_STATUS

    // //////////////////////////////////
    // 下面为其他状态数据, 调用者设定无效
    // //////////////////////////////////
    struct timeval      group_submit_time;        // 任务组提交时间点
    struct timeval      group_callback_time;      // 任务组回调时间点
    struct timeval      group_timeout_point;      // 任务组超时时间点

    LTasksGroup_T*      pre;
    LTasksGroup_T*      next;
    unsigned char       node_state;

    LTasksGroup_T() {
        m_valid_tasks = 0;
        memset(&m_user_data4, 0, sizeof(m_user_data4));
        m_user_data1 = m_user_data2 = m_user_data3 = m_user_data4;
        m_timeout_val = kDefaultTasksGroupTimeoutVal;
        node_state = NODE_STATE_IN_USE; // be careful: 如果用节点管理器，这个就不是in_use了
        __internal_ResetPtr();
    }

    uint32_t GetMaxTasks()const {
        return T_max_tasks;
    }

    void SetValidTasks(const uint32_t valid_tasks,
                       ENUM_TASKSGROUP_CALLBACK_STRATEGY tcs = STRATEGY_ALL_TASKS_MUST_FINISH,
                       const uint32_t need_finish_count = 0) {
        m_valid_tasks = (valid_tasks <= T_max_tasks) ?
                        valid_tasks :
                        T_max_tasks;
        m_strategy_info.strategy_type = tcs;
        switch (tcs) {
        case STRATEGY_ALL_TASKS_MUST_FINISH: {
            // TODO(lonely) 目前完全兼容老版本，
            //              后续修改策略或者增加策略需要考虑这里
        }
        break;
        case STRATEGY_N_TASKS_M_FINISH: {
            m_strategy_info.data.need_net_finished_count =
                (need_finish_count > m_valid_tasks)?
                m_valid_tasks:
                need_finish_count;
            if (m_valid_tasks == m_strategy_info.data.need_net_finished_count) {
                m_strategy_info.strategy_type = STRATEGY_ALL_TASKS_MUST_FINISH;
            }
        }
        break;
        case STRATEGY_SOME_TASK_NOT_MUST_FINISH: {
            m_strategy_info.data.need_finished_taskid_sum = 0;
            for (uint32_t idx = 0; idx < m_valid_tasks; ++idx) {
                if (m_tasks[idx].is_need_finish)
                    m_strategy_info.data.need_finished_taskid_sum += idx + 1;
            }
        }
        break;
        default:
            break;
        }
    }

    //
    // 缺省超时时间为1s
    //
    void SetTasksGroupTimeoutVal(const uint32_t time_out_ms) {
        m_timeout_val = time_out_ms;
    }

    // 只适用于检查回应包，对于请求包无效
    uint32_t HaveSomeValidTasks() const {
        return (m_valid_tasks>0 && __task_finished_count>0) ? 1:0;
    }

    //
    // 以下为系统设置状态接口，应用设置无效
    //
    void __internal_ResetPtr() {
        pre = next = NULL;
        __net_finished_count  = __task_finished_count = __valid_connections = 0;
        __need_callback_count = 0;
        __group_callback_status = STATUS_ALL_FINISH;
    }

    void __internal_SetGroupStatus() {
        struct timeval cur_time;
        lite_gettimeofday(&cur_time, NULL);

        // set stat item
        group_submit_time = cur_time;

        // set sys status
        __internal_ResetPtr();

        // set group timeout point
        group_timeout_point = cur_time;
        SetAbsTimeout(group_timeout_point, m_timeout_val);
    }
};



#define _MAX_LONGCONN_TASKS  256
typedef LTasksGroup_T<_MAX_LONGCONN_TASKS> LTasksGroup;

#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

// ////////////////////////
// Interface
// ////////////////////////

//
// interface:   ITasksGroupCallBack
// description: 1:任务组完成后回调返回
//              2:time out, error也从这个call back返回
//
interface ITasksGroupCallBack {
public:
    virtual void OnTasksFinishedCallBack(LTasksGroup* task_group) = 0;
    virtual void OnUserRequest(LongConnHandle session, // Long conn session
                                const unsigned char* data,
                                uint32_t len,       // received data, 只读
                                bool& need_response // 用于用户层设置该次
                                // 请求是否会有回应包,
                                //
                                // 用于缓减洪水般短连接
                                // 请求下的雪崩效应
                                //
                                // 默认值:
                                // need_response = true,
                                // 如果不想回应:
                                // 这次请求则设置
                                // need_response = false
                              ) = 0;
    virtual void OnClose(LongConnHandle lc_handle) = 0;

    ITasksGroupCallBack() {}
    virtual ~ITasksGroupCallBack() {}
};

//
// interface:   ILongConn
// description: Long connection interface
//
// interface ILongConn
class ILongConn {
public:
    //
    //  驱动收发对象工作
    //
    virtual void RoutineLongConn(uint32_t timeout_millisecs) = 0;

    //
    // ***独占模式***
    // 使用单独的接收线程的时候使用
    // max_sessions:    最多多少个长连接(connect to server和new accept)
    // listen_host:     listen host
    // port:            listen port
    // timeout:         send, recv time out. units:seconds.
    //                  最小可以支持0.001s.
    //                  例如0.2s = 200 milliseconds
    //                  todo(lonely):现在每个任务组可以设置时间
    //                  后面把这个参数删除
    //
    // break_epoll_wait:fast quit epoll set, 主控线程退出时候设定
    //
    virtual bool InitLongConn(ITasksGroupCallBack* callback,
                              uint32_t max_sessions,
                              const char* listen_host, uint16_t port, float timeout,
                              bool* break_epoll_wait = NULL,
                              CallBackStrategyConfig* callback_config = NULL) = 0;

    //
    // ***竞争模式***
    // 后面使用竞争模型的时候使用
    // 参数和上面一个Init相同, 外部bind on listen port,
    // 多个长连接模块竞争accept
    //
    virtual bool InitLongConn(ITasksGroupCallBack* callback,
                              uint32_t max_sessions,
                              SOCKET listen_sock, float timeout,
                              bool* break_epoll_wait = NULL,
                              CallBackStrategyConfig* callback_config = NULL) = 0;
    //
    // 发送数据
    // 返回值:
    //        true, 发送成功.通过CallBack传回TasksGroup各状态信息
    //        false, 发送失败.无任何CallBack回调
    //
    virtual bool SendData(LTasksGroup* task_group) = 0;

    //
    // 创建长连接session
    // LongConnHandle.handle = 0表示失败
    //
    virtual LongConnHandle CreateLongConnSession(const char* to_server,
            uint16_t port) = 0;

    //
    // 关闭长连接session
    //
    virtual void RemoveLongConnSession(LongConnHandle session) = 0;

    //
    // 接收用户新的连接
    // 当启用多线程的时候, 外部accept new socket fd, 并平均分配到每个线程中
    // 如果失败, 不会关闭socket, 需要调用者显示关闭socket
    //
    virtual bool InsertNewFD(SOCKET new_sock) = 0;

    //
    // 反初始化
    //
    virtual void UninitLongConn() = 0;

    //
    // Release me
    //
    virtual void Release() = 0;

    //
    // 返回长连接对方服务器ip, port
    //
    virtual bool GetPeerName(LongConnHandle session,
                             uint32_t* host,
                             uint16_t* port) = 0;

    //
    // 返回Long connection session对应的socket handle
    //
    virtual SOCKET GetSockHandle(LongConnHandle session) = 0;

    //
    // 强行退出当前epoll_wait
    //
    virtual void ForceBreakCurrWait() = 0;

    //
    // 得到CEpoll中Notify Socket Pair中的write peer,
    // 可以配合别的网络框架做异步处理
    //
    virtual void GetWritePeerOfNotify(SOCKET** sock) = 0;
    // 清空Epoll消息队列
    virtual void CleanUpNotifyMessages() = 0;

    ILongConn() {}
    virtual ~ILongConn() {}
};

//
// function:
// description: create ILongConn object
//
ILongConn* CreateLongConnObj(void);

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_INTERFACE_LONGCONN_H_
