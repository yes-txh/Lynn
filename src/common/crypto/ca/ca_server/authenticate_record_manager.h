// 该类对用户认证的记录信息进行管理

#ifndef COMMON_CRYPTO_CA_CA_SERVER_AUTHENTICATE_RECORD_MANAGER_H_
#define COMMON_CRYPTO_CA_CA_SERVER_AUTHENTICATE_RECORD_MANAGER_H_

#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/crypto/ca/ca_server/dir_manage.h"

namespace ca {

class AuthRecord
{
public:
    // 静态成员函数,提供全局访问的接口
    static AuthRecord* GetInstance() {
        if (NULL == s_authenticate_record) {
            CXThreadAutoLock auto_lock(&s_mutex);
            if (NULL == s_authenticate_record) {
                s_authenticate_record = new AuthRecord();
                s_authenticate_record->Init();
            }
        }
        return s_authenticate_record;    
    }

    static void FreeInstance() {
        if (s_authenticate_record) {
            CXThreadAutoLock auto_lock(&s_mutex);
            if (s_authenticate_record) {
                delete s_authenticate_record;
                s_authenticate_record = NULL;
            }
        }
    }

    // 追加记录
    // 对于认证的identity和对应时间的记录，每分钟追加写一次,格式为[time]--identity ip
    // 对于认证次数的统计，每小时追加写一次，格式为huor=count
    bool AppendRecord(const char* identity, const char* client_ip);

    // 读取本地文件
    // req_chart_epoch_time:        指定要读取哪一天的认证统计和认证记录文件
    // req_day_num:           从req_chart_epoch_time那天算最近的几天，例req_day_num=7，则读取最近7天的记录
    // req_page_num:          默认为0，不显示。认证记录的内容只显示一天的，且分页显示，默认每页显示1万条
    // total_page_num         可以返回被读取的文件共可以分为多少页
    // count_statistics_file: 返回指定日期内认证计数统计的文件内容
    // auth_record_file:      返回指定日期内认证记录的文件内容
    bool ReadRecord(time_t req_chart_epoch_time, time_t req_text_epoch_time, int32_t req_day_num,
                    int32_t req_page_num, int32_t* total_page_num, std::string* auth_record_file,
                    std::string* count_statistics_file);

private:
    // 单态模式，对唯一实例的受控访问
    AuthRecord(){};
    ~AuthRecord(){};
    
    // 生成用户认证记录文件存储目录
    bool Init();

private:
    // 静态成员变量,提供全局惟一的一个实例
    static AuthRecord* s_authenticate_record;

    // 互斥锁，处理多线程下的单态模式
    static CXThreadMutex s_mutex;

    DirManage m_dir_manage;
    
    char m_auth_record_dir[MAX_PATH]; // 认证记录文件存储的根目录
    char m_record_dir[MAX_PATH]; // 存储记录文件的目录
    char m_counter_dir[MAX_PATH]; // 存储统计文件的目录
    char m_auth_record_file_name[MAX_PATH]; // 存储认证的identity和对应时间的文件
    char m_count_record_file_name[MAX_PATH]; // 存储认证次数统计的文件

    // 每天新建一个auth_record_file文件记录identity和对应的认证时间，每分钟追加写一次
    // 每天新建一个count_statistics_file文件记录认证次数的统计，每小时追加写一次
    struct tm m_last_minute_time;
    struct tm m_last_hourly_time;

    std::string m_minute_auth_record; // 一分钟内认证的identity和对应的时间
    int32_t m_hourly_count; // 一小时内认证次数统计    
};

} // namespace ca
#endif // COMMON_CRYPTO_CA_CA_SERVER_AUTHENTICATE_RECORD_MANAGER_H_