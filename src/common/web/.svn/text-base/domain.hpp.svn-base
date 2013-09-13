/**
 * @file domain.hpp
 * @brief 对域名操作的封装。主要提供如下功能：
 *        1.计算主域
 *        2.计算下载调度子域
 *        3.计算相关性聚合后子域
 *        4.判断域名是否合法
 *        5.获取域名后缀
 *
 *        此类需要域名后缀，下载调度子域后缀，相关性聚合子域后缀三类配置数据的支持。
 *        这三类配置数据即可以从本地文件加载，也可以从服务器加载。
 *        默认的服务器地址 172.24.28.218 50690
 *
 *        为了便利性，此类不依赖于其他文件。仅支持 linux。
 *        
 * @author alexdu
 * @version 2.0
 * @date 2011-03-31
 */
#ifndef COMMON_DOMAIN_HPP
#define COMMON_DOMAIN_HPP

#include<string>
#include<vector>
#include<list>

namespace websearch {
namespace domain {

class Domain
{
public:
    /**
     * @brief 从网络初始化 domain 类，从域名后缀服务器主动拉取配置数据。做为一个长时间运行的服务器
     *        为了保证配置文件最新，使用者一定要定时主动调用 Reload 函数，以保证配置文件最新。
     *        因配置文件约一天更新一次，建议每一小时调用一次 Reload() 函数。
     *
     * @param ip                            域名后缀服务器 ip 地址，具体请咨询 alexdu
     * @param port                          域名后缀服务器端口，具体请咨询 alexdu
     * @param need_aggre_domain             是否需要下载组调度域名后缀
     * @param need_rel_aggre_domain         是否需要相关性聚合子域后缀
     * @return true                         初始化成功
     *         false                        初始化失败，说明加载的文件有问题，调用者不能再继续使用
     *                                      domain 类
     **/
    static bool Initialize(const char* ip, 
            unsigned short port,
            bool need_aggre_domain,
            bool need_rel_aggre_domain);

    /**
     * @brief 从本地文件初始化 domain 类。强烈不建议使用该函数，建议使用从网络加载数据的接口。
     *        除非你确保不需要最新的配置文件也可完成任务。
     *
     * @param domain_suffix_file            域名后缀文件名，不能为 NULL
     * @param aggre_domain_suffix_file      下载调度域名后缀文件名，如果不需要，可以为 NULL
     * @param rel_aggre_domain_suffix_file  相关性聚合域名后缀文件名，如果不需要，可以为 NULL
     * @return true                         初始化成功
     *         false                        初始化失败，说明加载的文件有问题，调用者不能再继续使用
     *                                      domain 类
     **/
    static bool Initialize(const char* domain_suffix_file,
            const char* aggre_domain_suffix_file,
            const char* rel_aggre_domain_suffix_file);
    
    
    /**
     * @brief 通过网络更新配置文件，适合于长期运行的 server。
     *
     * @param 
     * @return true  更新配置文件成功
     *         false 更新配置文件失败
     * 
     **/
    static bool Reload();

    /**
     * @brief domain 类的构造函数
     *
     * @param domain 子域(可包含端口号)，不能带 http:// 等url内容。
     * @return       无
     * 
     **/
    Domain(const char* domain);

    /**
     * @brief 判断一个域名是否合法
     *
     * @param 
     * @return true  合法域名
     *         false 非法域名
     * 
     **/
    bool IsValidDomain();

    /**
     * @brief 计算一个子域的主域。如 www.sina.com.cn，可得到 sina.com.cn
     *
     * @param 
     * @return 返回指向主域的指针位置，此指针只读
     * 
     **/
    char* GetDomain();

    /**
     * @brief 得到下载调度聚合后子域
     *
     * @param  sche_subdomain   引用指针，指向聚合后的域名，调用者不能释放
     * @return true  该子域是聚合域名
     *         false 该子域与聚合无关
     * 
     **/
    bool GetScheSubdomain(char*& sche_subdomain);

    /**
     * @brief 得到相关性聚合后子域
     *
     * @param  sche_subdomain   引用指针，指向聚合后的域名，调用者不能释放
     * @return true  该子域是聚合域名
     *         false 该子域与聚合无关
     * 
     **/
    bool GetRelScheSubdomain(char*& sche_subdomain);

#ifdef USE_MD5
    /**
     * @brief 得到子域 id
     *
     * @param  
     * @return 子域id
     *         
     * 
     **/
    unsigned long long GetSubdomainID() const;

    /**
     * @brief 得到主域 id
     *
     * @param  
     * @return 主域id
     *         
     * 
     **/
    unsigned long long GetDomainID() const;
#endif

    /**
     * @brief 得到一个域名的后缀
     *
     * @param 
     * @return 指向域名后缀的指针。如对于域名 news.qq.com，返回指针指向 .com
     * 
     **/
    char* GetDomainSuffix();
    
    static void GetReverseSubDomain(char* dest, const char* src, int len);
    static void GetReverseSubDomain(char* dest, const char* src);

private:
    enum DataLoadMethod
    {
        FROM_LOCAL_FILE = 0,
        FROM_NETWORK = 1
    };

    enum DomainEnum
    {
        MAX_DOMAIN_LENGTH = 50,
        MAX_HASH_SIZE = 65535
    };


    enum MessageType
    {
        GET_DOMAIN_SUFFIX = 1,
        GET_AGGRE_DOMAIN = 2,
        GET_REL_AGGRE_DOMAIN = 3
    };

#pragma pack(1)    
    struct SpiderPkg    
    {       
        unsigned char bVer;    
        int iBodyLen;
        int iCheckSum;  
        unsigned short wSrc;   
        unsigned short wDest;
        unsigned int dwSeqNo;  
        unsigned short wCmd;     
        unsigned char bIsTest;
        unsigned int dwTestSessionID; 
        unsigned int dwMsgType;  
    }; 
#pragma pack()

    class AutoLocker
    {
    public:
        AutoLocker(pthread_mutex_t* mutex) : m_mutex(mutex)
        {
            pthread_mutex_lock(m_mutex); 
        }

        ~AutoLocker()
        {
            pthread_mutex_unlock(m_mutex);
        }

        pthread_mutex_t* m_mutex;
    };

    char m_subdomain[MAX_DOMAIN_LENGTH + 7];
    char m_domain[MAX_DOMAIN_LENGTH + 1];
    char m_sche_subdomain[MAX_DOMAIN_LENGTH + 1];

    unsigned int m_subdomain_len;

    int  m_suffix_pos;

    int  m_domain_pos;
    int  m_port_pos;
    bool m_bIP;

    typedef std::vector< std::vector<std::string> > HashMap;

    static HashMap m_domain_suffix;

    static HashMap m_not_aggre_subdomain;
    static HashMap m_aggre_domain_suffix;

    static HashMap m_rel_not_aggre_subdomain;
    static HashMap m_rel_aggre_domain_suffix;

    static unsigned int GetHash(const char* str, int len);

    bool StrInHashMap(const char* str, int len, const HashMap& hash_map) const;

    void Parse(const char* domain);
    bool IsValidDomainChar(char c);

    static bool GetDataFromServer(MessageType msg, std::list<std::string>* result);
    static bool InitSocket(int fd);

    static int SendTcpData(int fd, char* buffer, size_t size);
    static int RecvTcpData(int fd, char* buffer, size_t size);
    static bool LoadDataFromFile(const char* file, std::list<std::string>* data);
    
    static void SetDomainSuffix(const std::list<std::string>& data);
    static void SetAggreDomainSuffix(const std::list<std::string>& data);
    static void SetRelAggreDomainSuffix(const std::list<std::string>& data);
    
    static std::string m_ip;
    static unsigned short m_port;
    static DataLoadMethod m_data_load_method;
    static bool m_need_aggre_domain;
    static bool m_need_rel_aggre_domain;
    static pthread_mutex_t m_mutex;
    static std::string GetCurTime();
    static void WriteLog(const std::string& strFile, unsigned int dwLine, const std::string& strLog);

};
};
};
#endif
