#include <stdio.h>
#include "thirdparty/glog/logging.h"
#include "thirdparty/gflags/gflags.h"
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/crypto/ca/ca_server/ca_server_performance_test/trans_data_thread.h"

bool g_is_shut_down = false;
const uint32_t kMaxFdLimit = 65535;
const char* const kServerIp = "172.26.1.184";
const uint16_t kServerPort = 30082;
const int32_t kThreadNum = 1;


// 连接数
DEFINE_int32(conn_num, 1, "并发连接数");
// 线程数
// DEFINE_int32(thread_num, 1, "线程数");


void DealSigInt(int32_t sig) {
    LOG(WARNING)  << "catch a termination single,sig number " << sig;
    g_is_shut_down = true;
}


DECLARE_USING_LOG_LEVEL_NAMESPACE
//设置日志级别
void LogLevelSettings() {
#ifdef _DEBUG
    InitGoogleDefaultLogParam(0);
    FLAGS_minloglevel = 0;
    FLAGS_stderrthreshold = 0;
#else
    InitGoogleDefaultLogParam(4);
    FLAGS_minloglevel = 4;
    FLAGS_stderrthreshold = 4;
#endif
}

template <typename ElemType>
void GetMin(ElemType* array, int32_t len, ElemType* min) {
    int32_t i;
    *min = array[0];
    for (i = 1; i < len; i++) {
        if (array[i] < *min)
            *min = array[i];
    }
}

template <typename ElemType>
void GetMax(ElemType* array, int32_t len, ElemType* max) {
    int32_t i;
    *max = array[0];
    for (i = 1; i < len; i++) {
        if (array[i] > *max)
            *max = array[i];
    }
}

int main(int argc, char** argv) {
    LogLevelSettings();
    AutoBaseLib auto_baselib;

    //解析命令行参数
    google::AllowCommandLineReparsing();
    if ( static_cast<uint32_t>(1) != google::ParseCommandLineFlags(&argc, &argv, true)) {
        LOG(ERROR) << "google::ParseCommandLineFlags return false";
        return -1;
    }
    printf ("conn_num  = %d\r\n", FLAGS_conn_num);
    printf ("thread_num = %d\r\n", kThreadNum);

    // 处理Ctrl+C 消息,15 SIGTERM
    signal(SIGINT, &DealSigInt);
    signal(SIGTERM, &DealSigInt);
    SetFDLimit(kMaxFdLimit);
    // set core limit
    SetCoreLimit();

    CXSocketLibAutoManage auto_socket_lib_manage;
    CTransDataThread obj_trans_threads[kThreadNum];

    for (int32_t i = 0; i < kThreadNum; i++) {
        // connect to server
        // 单线程与服务器的连接有conn_num个，启动thread_num个线程，共thread_num*conn_num个连接
        // 如果连接服务器成功，则将socket句柄注册到epoll中
        if (obj_trans_threads[i].Init(kServerIp, kServerPort, FLAGS_conn_num)) {
            // 启动线程，routine
            obj_trans_threads[i].StartThread();
        }
    }

    uint32_t send_count_per_minute_pre = 0;
//    uint32_t send_count_fail_per_minute_pre = 0;
    uint32_t ok_count_per_minute_pre = 0;
    uint32_t fail_count_per_minute_pre = 0;
    uint32_t connect_count_per_minute_pre = 0;
    uint32_t epoll_error_count_per_minute_pre = 0;
    uint32_t send_count_per_minute_now = 0;
//    uint32_t send_count_fail_per_minute_now = 0;
    uint32_t ok_count_per_minute_now = 0;
    uint32_t fail_count_per_minute_now = 0;
    uint32_t connect_count_per_minute_now = 0;
    uint32_t epoll_error_count_per_minute_now = 0;
    uint64_t min_cost_time = 0;
    uint64_t max_cost_time = 0;
    uint64_t aver_cost_time = 0;
    uint64_t min_cost_time_array[kThreadNum];
    uint64_t max_cost_time_array[kThreadNum];

    while (!g_is_shut_down) {
        for (int32_t i = 0; i < kThreadNum; ++i) {
            ok_count_per_minute_now += obj_trans_threads[i].GetOkCount();
            fail_count_per_minute_now += obj_trans_threads[i].GetFailCount();
            send_count_per_minute_now += obj_trans_threads[i].GetSendCount();
          //  send_count_fail_per_minute_now += obj_trans_threads[i].GetSendFailCount();
            connect_count_per_minute_now += obj_trans_threads[i].GetConnectCount();
            epoll_error_count_per_minute_now += obj_trans_threads[i].GetEpollErrorCount();
            aver_cost_time += obj_trans_threads[i].GetAverCostTime();
            min_cost_time_array[i] = obj_trans_threads[i].GetMinCostTime();
            max_cost_time_array[i] = obj_trans_threads[i].GetMaxCostTime();
        }
        GetMax(max_cost_time_array, kThreadNum, &max_cost_time);
        GetMin(min_cost_time_array, kThreadNum, &min_cost_time);
        printf ("send_count_per_minute = %d\r\n",
                send_count_per_minute_now - send_count_per_minute_pre);
        // printf ("send_count_fail_per_minute = %d\r\n",
        //        send_count_fail_per_minute_now - send_count_fail_per_minute_pre);
        printf ("ok_count_per_minute = %d\r\n",
                ok_count_per_minute_now - ok_count_per_minute_pre);
        printf ("fail_count_per_minute = %d\r\n",
                fail_count_per_minute_now - fail_count_per_minute_pre);
        printf ("send_count_per_second = %d\r\n",
               (send_count_per_minute_now - send_count_per_minute_pre)/ 60);
        // printf ("send_count_fail_per_second = %d\r\n",
        //      (send_count_fail_per_minute_now - send_count_fail_per_minute_pre) / 60);
        printf ("ok_count_per_second = %d\r\n",
               (ok_count_per_minute_now - ok_count_per_minute_pre)/ 60);
        printf ("fail_count_per_second = %d\r\n",
               (fail_count_per_minute_now - fail_count_per_minute_pre) / 60);
        printf ("connect_count_per_second = %d\r\n",
               (connect_count_per_minute_now - connect_count_per_minute_pre)/ 60);
        printf ("epoll_error_count_per_second = %d\r\n",
               (epoll_error_count_per_minute_now - epoll_error_count_per_minute_pre)/ 60);
        printf ("average cost time per response = %lu(microseconds)\r\n",
                aver_cost_time / kThreadNum);
        printf ("max cost time per response = %lu(microseconds)\r\n", max_cost_time);
        printf ("min cost time per response = %lu(microseconds)\r\n", min_cost_time);
        printf ("\r\n");

        send_count_per_minute_pre = send_count_per_minute_now;
        ok_count_per_minute_pre = ok_count_per_minute_now;
        fail_count_per_minute_pre = fail_count_per_minute_now;
        connect_count_per_minute_pre = connect_count_per_minute_now;
        epoll_error_count_per_minute_pre = epoll_error_count_per_minute_now;
        send_count_per_minute_now = 0;
        ok_count_per_minute_now = 0;
        fail_count_per_minute_now = 0;
        connect_count_per_minute_now = 0;
        epoll_error_count_per_minute_now = 0;
        aver_cost_time = 0;

        // 驱动线程运行
        XSleep(60*1000);
    }

    // uninit
    for (int32_t i = 0; i < kThreadNum; i++) {
        obj_trans_threads[i].EndThread();
    }

    return 0;
}
