#ifndef COMMON_BASELIB_SVRPUBLIB_BINARY_LOG_H_
#define COMMON_BASELIB_SVRPUBLIB_BINARY_LOG_H_

// 写二进制日志，主要是由base_rotocol的CBaseProtocolPack和CBaseProtocolUnPack
// 中调用，把通信过程中的数据包记录在日志文件中，其中
//
// 二进制日志默认是关闭的,如果需要打开二进制日志，则在main函数之前声明
// DECLARE_bool(binary_log);
// 然后在main函数中FLAGS_binary_log=true;
//
// 默认的日志文件最大大小为20M，如果需要设置日志文件的最大大小
// 则在main函数之前声明
// DECLARE_int32(binary_log_size);
// 然后在main函数中FLAGS_binary_log_size=xxxxxxx;
//
// 默认的保留10个日志文件，即日志循环写在这10个文件中，如果需要修改日志文件个数
// 则在main函数之前声明
// DECLARE_int32(binary_log_count);
// 然后在main函数中FLAGS_binary_log_count=xxxxxxx;
//

#include <stdio.h>
#include <string>
#include <fstream>
#include "common/baselib/svrpublib/twse_type_def.h"
#include "common/baselib/svrpublib/server_publib.h"


DECLARE_int32(binary_log_size);
DECLARE_int32(binary_log_count);

_START_XFS_BASE_NAMESPACE_

#define WriteBinaryLog(content,len) g_binary_log->Write(content,len)

enum kBinaryLogHeadType {
    kVersion1 = 1,
};

enum kBaekBinaryLogHeadMagic {
    kMagic = 0x47852104,
};

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif //
struct BinaryLogHead {
    uint8_t  type;
    uint32_t magic;
    uint32_t tv_sec;
    uint32_t tv_usec;
    uint32_t len;        // 数据总长度，包括BinaryLogHead
    uint32_t reserve_len; // Head和Data直接可能含有reserve数据
    BinaryLogHead() {
        type = kVersion1;
        magic = kMagic;
        struct timeval tv = {0};
        lite_gettimeofday(&tv, 0);
        tv_sec = tv.tv_sec;
        tv_usec = tv.tv_usec;
        len = 0;
        reserve_len = 0;
    }

    bool IsValid() {
        return magic == kMagic && len >= sizeof(BinaryLogHead);
    }
};
#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif //

template<typename T>
class BinaryLog_ {
public:
    BinaryLog_();
    ~BinaryLog_();
    bool Write(const char* content, uint32_t content_len);
private:
    CXThreadMutex m_mutex;

    uint32_t m_cur_file_len;
    uint32_t m_log_file_index;
    std::string m_base_file;
    FILE*       m_log_writer;

    std::string BaseFileName();
    std::string BinaryLogFileName(int32_t index);
    void OpenNewBinaryFile();
    bool GetUserName(char* buff, int32_t len);
    bool GetLocalHostName(char* buff, int32_t len);
};

template<typename T>
BinaryLog_<T>::BinaryLog_():m_cur_file_len(0),
    m_log_file_index(0),
    m_log_writer(0) {
    m_base_file = BaseFileName();
}

template<typename T>
BinaryLog_<T>::~BinaryLog_() {
    if(m_log_writer)
        fclose(m_log_writer);
    m_log_writer = NULL;
}

template<typename T>
bool BinaryLog_<T>::Write(const char *content, uint32_t content_len) {
    CXThreadAutoLock auto_lock(&m_mutex);

    BinaryLogHead head;
    head.len += content_len;
    if (!m_log_writer)
        OpenNewBinaryFile();
    size_t ret = fwrite(&head, (int32_t)sizeof(head), 1, m_log_writer);
    if(ret != 1) {
        fclose(m_log_writer);
        m_log_writer = NULL;
        ++m_log_file_index;
        return false;
    }

    ret = fwrite(content, content_len, 1, m_log_writer);
    if (ret != 1) {
        fclose(m_log_writer);
        m_log_writer = NULL;
        m_log_file_index = (m_log_file_index+1) % FLAGS_binary_log_count;
        return false;
    }

    m_cur_file_len += head.len;

    if (m_cur_file_len > static_cast<uint32_t>(FLAGS_binary_log_size)) {
        m_log_file_index = (m_log_file_index+1) % FLAGS_binary_log_count;
        fclose(m_log_writer);
        m_log_writer = NULL;
        OpenNewBinaryFile();
    }
    return true;
}

template<typename T>
std::string BinaryLog_<T>::BaseFileName() {
    LogParam log_param;
    std::string file_name = log_param.m_log_module_name;

    char host_name_buff[256] = {0};
    GetLocalHostName(host_name_buff, sizeof(host_name_buff));

    // user name
    char user_name_buff[256] = {0};
    GetUserName(user_name_buff, sizeof(user_name_buff));

    file_name += '.';
    file_name += host_name_buff;

    file_name += '.';
    file_name += user_name_buff;

    file_name += '.';
    file_name += "binary_log";
    return file_name;
}

template<typename T>
std::string BinaryLog_<T>::BinaryLogFileName(int32_t index) {

    std::string file_name = m_base_file;
    char tmp[20] = {0};
    safe_snprintf(tmp, sizeof(tmp),"%02d.dat",index);
    file_name += tmp;
    return file_name;
}

template<typename T>
void BinaryLog_<T>::OpenNewBinaryFile() {
    if(m_log_writer)
        fclose(m_log_writer);
    m_log_writer = fopen( BinaryLogFileName(m_log_file_index).c_str(),"wb");
    m_cur_file_len = 0;

}

//
//  获取用户名字
//
template<class T>
bool BinaryLog_<T>::GetUserName(char* buff, int32_t len) {
    if (!buff)
        return false;

#ifdef WIN32
    char* user = getenv("USERNAME");
#else // linux
    char* user = getenv("USER");
#endif //
    if (user)
        safe_snprintf(buff, len, "%s", user);
    else
        safe_snprintf(buff, len, "%s", "invalid-user");
    return true;
}

//
//  获取本机计算机名字
//
template<typename T>
bool BinaryLog_<T>::GetLocalHostName(char* buff, int32_t len) {
    if (!buff)
        return false;
    memset(buff, 0, len);

#ifdef WIN32
    DWORD ret_len = len;
    if (!GetComputerNameA((LPTSTR)buff, &ret_len)) {
        safe_snprintf(buff, len, "unknown");
    }
#else // linux
    struct utsname uname_buf;
    if (uname(&uname_buf) == 0)
        safe_snprintf(buff, len, "%s", uname_buf.nodename);
    else
        safe_snprintf(buff, len, "%s", "unknown");
#endif //
    return true;
}

extern BinaryLog_<void>* g_binary_log;
_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_BINARY_LOG_H_
