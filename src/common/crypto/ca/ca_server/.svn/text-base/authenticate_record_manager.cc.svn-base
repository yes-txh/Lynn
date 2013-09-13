#include <vector>
#include "common/crypto/ca/ca_server/authenticate_record_manager.h"
#include "common/crypto/ca/ca_public/ca_struct.h"
// #include "common/system/io/file_utility.hpp"
#include "common/baselib/svrpublib/key_value_parser.h"

DEFINE_int32(record_line, 1000, "record line numbers displayed on web page"); // 每页显示记录的条数

namespace ca {

const char* const kAuthenticateRecordDirName = "authenticate_record";
const char* const kRecordDirName = "record";
const char* const kCountDirName = "count";

// 类的静态成员变量要在类外进行定义
AuthRecord* AuthRecord::s_authenticate_record = NULL;
CXThreadMutex AuthRecord::s_mutex;

bool AuthRecord::Init()
{
    // 建立目录
    GetModuleFileName(NULL, m_auth_record_dir, sizeof(m_auth_record_dir));

    // linux and windows
    char* p = strrchr(m_auth_record_dir, '/');
    if ( !p )
        p = strrchr(m_auth_record_dir, '\\');

    if (NULL == p)
        return false;

    ++p;

    // 追加目录htdocs
    int32_t len_remain = m_auth_record_dir + sizeof(m_auth_record_dir) - p - 1;
    // 边界检查
    int32_t len_add_dir = strlen(kApacheHtdocsName) + strlen(kAuthenticateRecordDirName) + 2;
    if (len_remain < len_add_dir)
        return false;

    // 形成目录路径为：/xxx/xxx/htdocs
    safe_snprintf(p, len_remain, kApacheHtdocsName);
    p += strlen(kApacheHtdocsName);

    // 追加目录authenticate_record，形成目录路径为：/xxx/xxx/htdocs/authenticate_record
    len_remain = m_auth_record_dir + sizeof(m_auth_record_dir) - p - 1;
    safe_snprintf(p, len_remain, "%s%s", SPLIT_SIGN, kAuthenticateRecordDirName);
    if (!m_dir_manage.IsDirExist(m_auth_record_dir)) {
        m_dir_manage.MkDir(m_auth_record_dir);
    }

    p += strlen(kAuthenticateRecordDirName) + 1;

    // 追加目录record和count,形成目录路径为：
    // /xxx/xxx/htdocs/authenticate_record/record
    // /xxx/xxx/htdocs/authenticate_record/count
    len_remain = m_auth_record_dir + sizeof(m_auth_record_dir) - p - 1;
    safe_snprintf(p, len_remain, "%s%s", SPLIT_SIGN, kRecordDirName);
    if (!m_dir_manage.IsDirExist(m_auth_record_dir)) {
        m_dir_manage.MkDir(m_auth_record_dir);
    }
    safe_snprintf(m_record_dir, MAX_PATH, "%s", m_auth_record_dir);

    safe_snprintf(p, len_remain, "%s%s", SPLIT_SIGN, kCountDirName);
    if (!m_dir_manage.IsDirExist(m_auth_record_dir)) {
        m_dir_manage.MkDir(m_auth_record_dir);
    }
    safe_snprintf(m_counter_dir, MAX_PATH, "%s", m_auth_record_dir);

    // 初始化时间和内存记录
    m_hourly_count = 0;
    m_minute_auth_record.clear();
    time_t current_epoch_time = time(NULL);
    safe_localtime(&current_epoch_time, &m_last_hourly_time);
    safe_localtime(&current_epoch_time, &m_last_minute_time);

    return true;
}

// 追加记录
// 对于认证的identity和对应时间的记录，每分钟追加写一次,格式为[time]--identity ip
// 对于认证次数的统计，每小时追加写一次，格式为huor=count
bool AuthRecord::AppendRecord(const char* identity, const char* client_ip)
{
    CXThreadAutoLock auto_lock(&s_mutex);
    if (!identity)
        return false;

    time_t current_epoch_time = time(NULL);
    struct tm current_local_time;
    safe_localtime(&current_epoch_time, &current_local_time);

    // 格式化记录, 格式为[time]--identity ip
    char current_gmt_time[128];
    strftime(current_gmt_time, sizeof(current_gmt_time), "%Y-%m-%d %H:%M:%S", &current_local_time);
    char auth_record[128];
    safe_snprintf(auth_record, sizeof(auth_record),
        "[%s]--%s %s", current_gmt_time, identity, client_ip);

    // 记录identity和对应时间,每分钟追加写一次
    if (current_local_time.tm_year == m_last_minute_time.tm_year
        && current_local_time.tm_yday == m_last_minute_time.tm_yday
        && current_local_time.tm_min == m_last_minute_time.tm_min) {
            m_minute_auth_record = m_minute_auth_record + auth_record + "\n";
    } else {
        // 把之前的记录追加写入文件里,文件名诸如：2011_6_24
        safe_snprintf(m_auth_record_file_name, MAX_PATH, "%s%s%d_%d_%d", m_record_dir,
                      SPLIT_SIGN, m_last_minute_time.tm_year + 1900, m_last_minute_time.tm_mon + 1,
                      m_last_minute_time.tm_mday);
        m_dir_manage.WriteFile(m_auth_record_file_name, "%s", m_minute_auth_record.c_str());
        m_minute_auth_record.clear();
        m_minute_auth_record = m_minute_auth_record + auth_record + "\n";
        m_last_minute_time = current_local_time;
    }


    // 记录认证次数,每小时追加写一次
    if (current_local_time.tm_year == m_last_hourly_time.tm_year
        && current_local_time.tm_yday == m_last_hourly_time.tm_yday
        && current_local_time.tm_hour == m_last_hourly_time.tm_hour) {
            m_hourly_count++;
    } else { // 追加写前一个小时的认证计数到文件，文件名诸如：2011_6_24, 格式为hour=count
        safe_snprintf(m_count_record_file_name, MAX_PATH, "%s%s%d_%d_%d", m_counter_dir,
                      SPLIT_SIGN, m_last_hourly_time.tm_year + 1900, m_last_hourly_time.tm_mon + 1,
                      m_last_hourly_time.tm_mday);
        char count_record[128];
        // 记录每小时用户认证次数，格式如：1=786
        safe_snprintf(count_record, sizeof(count_record), "%d=%d\n",
                      m_last_hourly_time.tm_hour, m_hourly_count);
        m_dir_manage.WriteFile(m_count_record_file_name, "%s", count_record);
        m_last_hourly_time = current_local_time;
        m_hourly_count = 1;
    }

    return true;
}


bool AuthRecord::ReadRecord(time_t req_chart_epoch_time, time_t req_text_epoch_time,
                            int32_t req_day_num, int32_t req_page_num, int32_t* total_page_num,
                            std::string* auth_record_file, std::string* count_statistics_file) {
    CXThreadAutoLock auto_lock(&s_mutex);

    struct tm req_local_time = {0};
    char req_day[128] = {0};
    time_t pre_req_epoch_time = 0;
    std::string single_day_count;
    CKeyValueParser key_value_parse_obj;


    time_t current_epoch_time = time(NULL);
    struct tm current_local_time;
    safe_localtime(&current_epoch_time, &current_local_time);

    // ---------------------------------------------------------------------------------------
    // 默认是不读取认证identity及对应时间的文件，即req_page_num=0. 可通过页面的按钮控制是否读取显示
    if (req_page_num > 0) {
        safe_localtime(&req_text_epoch_time, &req_local_time);
        safe_snprintf(req_day, sizeof(req_day), "%d_%d_%d", req_local_time.tm_year + 1900,
            req_local_time.tm_mon + 1, req_local_time.tm_mday);

        // 读取认证identity及对应时间的文件
        safe_snprintf(m_auth_record_file_name, MAX_PATH, "%s%s%s",
            m_record_dir, SPLIT_SIGN, req_day);
        int32_t total_line_num = 0;
        int32_t got_line_num = 0;
        int32_t req_start_line_num = (req_page_num - 1) * FLAGS_record_line + 1;
        int32_t req_end_line_num = req_page_num * FLAGS_record_line;
        *auth_record_file += m_dir_manage.ReadTextFile(m_auth_record_file_name, req_start_line_num,
                                                  req_end_line_num, &got_line_num, &total_line_num);

        // 如果要读取的时间是当天，则还需要处理内存中尚未写入文件的数据
        if (current_local_time.tm_year == req_local_time.tm_year
            && current_local_time.tm_yday == req_local_time.tm_yday) {
            // 如果从文件中读取到的行数小于FLAGS_record_line，则还需要从内存中读取
            if (got_line_num < FLAGS_record_line) {
                std::string::size_type idx = 0;
                std::string::size_type start_pos = 0;
                std::string::size_type stop_pos = std::string::npos;
                // 首先定位要求读取的起始位置
                // 如果已从文件中读取了一定行数，则从内存中数据的开始位置读取剩余所需的行数
                for (; idx < m_minute_auth_record.length(); ++idx) {
                    idx = m_minute_auth_record.find_first_of('\n', idx);
                    if (idx == std::string::npos)
                        break;
                    else {
                        ++total_line_num;
                        if ((total_line_num + 1) == req_start_line_num)
                            start_pos = idx + 1;
                        if (total_line_num == req_end_line_num)
                            stop_pos = idx + 1;
                    }
                }
                std::string::size_type substr_num = (stop_pos == std::string::npos) ? stop_pos :
                                                    (stop_pos - start_pos);
                if (req_start_line_num > total_line_num) {
                    char temp_data[128] = {0};
                    safe_snprintf(temp_data, sizeof(temp_data),
                        "no record on page[%d]", req_page_num);
                    *auth_record_file = temp_data;
                } else {
                    *auth_record_file += m_minute_auth_record.substr(start_pos, substr_num); 
                }
            }

        }
        *total_page_num = (total_line_num % FLAGS_record_line == 0)
                          ? total_line_num / FLAGS_record_line
                          : (total_line_num / FLAGS_record_line) + 1;
    }

    // ----------------------------------------------------------------
    // 读取认证统计文件
    for (int32_t i = req_day_num - 1; i >= 0; --i) {
        pre_req_epoch_time = req_chart_epoch_time - i * 24 * 60* 60;
        safe_localtime(&pre_req_epoch_time, &req_local_time);
        safe_snprintf(req_day, sizeof(req_day), "%d_%d_%d", req_local_time.tm_year + 1900,
                      req_local_time.tm_mon + 1, req_local_time.tm_mday);

        // 读取认证统计文件
        safe_snprintf(m_count_record_file_name, MAX_PATH, "%s%s%s",
                      m_counter_dir, SPLIT_SIGN, req_day);
        if (1 == req_day_num) { //如果要求只读一天的记录，读取并返回，不需要累加该天每个时段的记录
            *count_statistics_file = m_dir_manage.ReadTextFile(m_count_record_file_name);

            // 将内存中还没追加写入文件的内容一起读出
            if (current_local_time.tm_year == req_local_time.tm_year
                && current_local_time.tm_yday == req_local_time.tm_yday) {
                    char count_record[128];
                    safe_snprintf(count_record, sizeof(count_record), "%d=%d",
                        m_last_hourly_time.tm_hour, m_hourly_count);
                    *count_statistics_file = *count_statistics_file + count_record + "\n";
            }

            return true;
        }
        // 如果要求读取多天的记录，则需将每天的记录按小时累加
        if (key_value_parse_obj.ParserFromFile(m_count_record_file_name)) {
            char key_hour[8] = {0};
            int32_t daily_count = 0;
            for (int32_t i = 0; i < 24;  ++i) {
                safe_snprintf(key_hour, sizeof(key_hour), "%d", i);
                char value_hourly_count[32] = {0};
                if (key_value_parse_obj.GetValue(key_hour,
                                             reinterpret_cast<unsigned char*>(value_hourly_count),
                                             sizeof(value_hourly_count))) {
                    daily_count += ATOI(value_hourly_count);
                }
            }

            // 如果要求读取的最后一天是当天，则需将内存中还没追加写入文件的内容一起读出
            if (current_local_time.tm_year == req_local_time.tm_year
                && current_local_time.tm_yday == req_local_time.tm_yday) {
                    daily_count += m_hourly_count;
            }
            char str_daily_count[128] = {0};
            safe_snprintf(str_daily_count, sizeof(str_daily_count), "%d", daily_count);
            *count_statistics_file = *count_statistics_file + req_day + "="
                                    + str_daily_count + "\r\n";
        } else
            *count_statistics_file = *count_statistics_file + req_day + "=" + "" + "\r\n";
    }
    return true;
}

} // namespace ca
