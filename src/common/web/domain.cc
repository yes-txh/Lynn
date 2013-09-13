#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/time.h>
#include "domain.hpp"

#ifdef USE_MD5
#include "common/crypto/hash/md5.cpp"
#endif

using namespace std;
using namespace websearch::domain;

Domain::HashMap Domain::m_domain_suffix;
Domain::HashMap Domain::m_not_aggre_subdomain;
Domain::HashMap Domain::m_aggre_domain_suffix;
Domain::HashMap Domain::m_rel_not_aggre_subdomain;
Domain::HashMap Domain::m_rel_aggre_domain_suffix;
string Domain::m_ip = "";
unsigned short Domain::m_port = 0;
Domain::DataLoadMethod Domain::m_data_load_method = FROM_LOCAL_FILE;
bool Domain::m_need_aggre_domain = false;
bool Domain::m_need_rel_aggre_domain = false;
pthread_mutex_t Domain::m_mutex;

#define WRITE_LOG(str) {do {WriteLog(__FILE__, __LINE__, str);} while(0);}

Domain::Domain(const char* domain)
{
    m_subdomain[0] = '\0';
    m_domain[0] = '\0';
    m_sche_subdomain[0] = '\0';

    m_subdomain_len = 0;
    m_suffix_pos = -1;
    m_domain_pos = -1;
    m_port_pos = -1;
    m_bIP = true;

    Parse(domain);
}

void Domain::Parse(const char* domain)
{
    char* p = const_cast<char*>(domain);

    if (*p == '.' || *p == ':')
    {
        return;
    }

    unsigned int len = 0;
    int dot_pos[6];
    int dot_num = 0;
    int last_split_pos = -2;
    while (*p)
    {
        if (len >= MAX_DOMAIN_LENGTH + 6)
        {
            return;
        }

        if (m_port_pos == -1 && len > MAX_DOMAIN_LENGTH)
        {
            return;
        }

        if (!IsValidDomainChar(*p))
        {
            return;
        }

        if (*p == '.' || *p == ':')
        {
            if (last_split_pos == (len - 1))
            {
                return;
            }

            last_split_pos = len;
        }

        if (*p == '.')
        {
            dot_pos[dot_num++] = len;
            if (dot_num > 6)
            {
                return;
            }

            if (m_bIP && (dot_num > 3))
            {
                m_bIP = false;
            }
        }

        if ((m_port_pos > 0) && !(*p >= '0' && *p <= '9'))
        {
            return;
        }

        if (*p == ':')
        {
            if (m_port_pos > 0)
            {
                return;
            }
            m_port_pos = len;
            if (len >= MAX_DOMAIN_LENGTH)
            {
                return;
            }
        }

        if (m_bIP && !((*p >= '0' && *p <= '9') || (*p == '.') || (*p == ':')))
        {
            m_bIP = false;
        }

        char c = *p;
        if (c >= 'A' && c <= 'Z')
        {
            c = 'a' + c - 'A';
        }
        m_subdomain[len] = c;

        p++;
        len++;
    }

    char c = m_subdomain[len - 1];
    if (c == '.' || c == ':' || c == '-' || c == '_')
    {
        return;
    }

    if (len <= 3 || dot_num <= 0)
    {
        return;
    }

    if (m_bIP && ((dot_num != 3) || (len > 21)))
    {
        return;
    }

    if (m_port_pos > 0)
    {
        if ((len - m_port_pos - 1 > 5)
             || (atoi(domain + m_port_pos + 1) > 65535))
        {
            return;
        }
    }

    if (m_bIP)
    {
        char buffer[4];
        int begin = 0;
        int end = 0;
        for (int i = 0; i < 4; i++)
        {
            if (0 == i)
            {
                begin = 0;
                end = dot_pos[i];
            }
            if (i > 0 && i <= 2)
            {
                begin = dot_pos[i - 1] + 1;
                end = dot_pos[i];
            }
            else if (i == 3)
            {
                begin = dot_pos[i - 1] + 1;
                end = m_port_pos > 0 ? m_port_pos: len;
            }

            int ip_len = end - begin;

            if (ip_len > 3)
            {
                return;
            }

            memcpy(buffer, m_subdomain + begin, ip_len);
            buffer[ip_len] = '\0';

            if (atoi(buffer) > 255)
            {
                return;
            }
        }
    }

    m_subdomain_len = len;
    m_subdomain[len] = '\0';

    if (m_bIP)
    {
        m_suffix_pos = 0;
        m_domain_pos = 0;

        int domain_len = m_port_pos > 0 ? m_port_pos : m_subdomain_len;

        memcpy(m_domain, m_subdomain, domain_len);
        m_domain[domain_len] = '\0';

        return;
    }

    int check_begin = dot_num > 2 ? dot_num - 2 : 0;

    for (int i = check_begin; i < dot_num; i++)
    {
        int suffix_len = m_port_pos > 0 ? m_port_pos - dot_pos[i] : m_subdomain_len - dot_pos[i];

        if (StrInHashMap(m_subdomain + dot_pos[i], suffix_len, m_domain_suffix))
        {
            m_suffix_pos = dot_pos[i];
            break;
        }
    }

    for (int i = m_suffix_pos - 1; i > 0; i--)
    {
        if (m_subdomain[i] == '.')
        {
            m_domain_pos = i + 1;

            break;
        }
    }

    if (m_suffix_pos > 0)
    {
        if (m_domain_pos < 0)
        {
            m_domain_pos = 0;
        }

        int domain_len =
            (m_port_pos > 0 ? m_port_pos - m_domain_pos :
                m_subdomain_len - m_domain_pos);
        memcpy(m_domain, m_subdomain + m_domain_pos, domain_len);
        m_domain[domain_len] = '\0';
    }
}

unsigned int Domain::GetHash(const char *str, int len)
{
    const char *p = str;
    unsigned char b = 29;
    unsigned char a = 17;
    unsigned int hash = 0;

    for (int i = 0; i < len; i++)
    {
        hash = hash * a + (*p++);
        a *= b;
    }
    hash = hash * a + (p - str);

    return (hash & (MAX_HASH_SIZE - 1));
}

bool Domain::Initialize(const char* domain_suffix_file,
        const char* aggre_domain_suffix_file,
        const char* rel_aggre_domain_suffix_file)
{
    m_need_aggre_domain = FROM_LOCAL_FILE;
    m_need_aggre_domain = false;
    m_need_rel_aggre_domain = false;

    list<string> data;
    bool bRet = false;
    bRet = LoadDataFromFile(domain_suffix_file, &data);
    if (!bRet)
    {
        assert(0);
        return false;
    }

    SetDomainSuffix(data);

    if (aggre_domain_suffix_file)
    {
        m_need_aggre_domain = true;
        bRet = LoadDataFromFile(aggre_domain_suffix_file, &data);
        
        if (!bRet)
        {
            assert(0);
            return false;
        }

        SetAggreDomainSuffix(data);
    }

    if (rel_aggre_domain_suffix_file)
    {
        m_need_rel_aggre_domain = true;

        bRet = LoadDataFromFile(rel_aggre_domain_suffix_file, &data);
        
        if (!bRet)
        {
            assert(0);
            return false;
        }

        SetRelAggreDomainSuffix(data);
    }

    return true;
}

bool Domain::StrInHashMap(const char* str, int len, const HashMap& hash_map) const
{
    AutoLocker locker(&m_mutex);

    const vector<string>& vec = hash_map[GetHash(str, len)];
    for (size_t i = 0; i < vec.size(); i++)
    {
        int str_len = static_cast<int>(vec[i].length());
        int cmp_len = len > str_len ? str_len : len;
        if (strncmp(vec[i].c_str(), str, cmp_len) == 0)
        {
            return true;
        }
    }

    return false;
}
char* Domain::GetDomainSuffix()
{
    if (-1 == m_suffix_pos)
    {
        return NULL;
    }

    char* p = m_domain;
    while (*p != '.')
    {
        p++;
    }

    return p;
}

bool Domain::IsValidDomain()
{
    return (m_suffix_pos != -1);
}

char* Domain::GetDomain()
{
    return m_domain;
}

bool Domain::IsValidDomainChar(char c)
{
    if ((c == '.')
        || (c == '-')
        || (c == '_')
        || (c == ':')
        || (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9'))
    {
        return true;
    }

    return false;
}

bool Domain::GetScheSubdomain(char*& sche_subdomain)
{
    memcpy(m_sche_subdomain, m_subdomain, m_subdomain_len + 1);
    sche_subdomain = m_sche_subdomain;
    if (-1 == m_domain_pos)
    {
        return false;
    }
    else
    {
        if (4 == m_domain_pos && strncmp("www.", m_subdomain, 4) == 0)
        {
            return false;
        }
    }

    int min_true_pos = -1;

    for (int i = m_domain_pos - 1; i > 0; i--)
    {
        if (m_subdomain[i] == '.')
        {
            int len = m_subdomain_len - i;
            if (StrInHashMap(m_subdomain + i, len, m_aggre_domain_suffix))
            {
                min_true_pos = i;
            }
        }
    }

    if (-1 == min_true_pos)
    {
        return false;
    }
    else
    {
        if (StrInHashMap(m_subdomain, m_subdomain_len, m_not_aggre_subdomain))
        {
            return false;
        }
        else
        {
            const char* p = "sosospider";
            memcpy(m_sche_subdomain, p, 10);
            memcpy(m_sche_subdomain + 10, m_subdomain + min_true_pos,
                   m_subdomain_len - min_true_pos + 1);

            return true;
        }
    }

    return false;
}

bool Domain::GetRelScheSubdomain(char*& sche_subdomain)
{
    memcpy(m_sche_subdomain, m_subdomain, m_subdomain_len + 1);
    sche_subdomain = m_sche_subdomain;
    if (-1 == m_domain_pos)
    {
        return false;
    }
    else
    {
        if (4 == m_domain_pos && strncmp("www.", m_subdomain, 4) == 0)
        {
            return false;
        }
    }

    int min_true_pos = -1;

    for (int i = m_domain_pos - 1; i > 0; i--)
    {
        if (m_subdomain[i] == '.')
        {
            int len = m_subdomain_len - i;
            if (StrInHashMap(m_subdomain + i, len, m_rel_aggre_domain_suffix))
            {
                min_true_pos = i;
            }
        }
    }

    if (-1 == min_true_pos)
    {
        return false;
    }
    else
    {
        if (StrInHashMap(m_subdomain, m_subdomain_len, m_rel_not_aggre_subdomain))
        {
            return false;
        }
        else
        {
            memcpy(m_sche_subdomain, m_subdomain + min_true_pos,
                   m_subdomain_len - min_true_pos + 1);

            return true;
        }
    }

    return false;
}

bool Domain::Initialize(const char* ip,
        unsigned short port,
        bool need_aggre_domain,
        bool need_rel_aggre_domain)
{
    if (NULL == ip || 0 == port)
    {
        return false;
    }

    m_ip = ip;
    m_port = port;
    m_data_load_method = FROM_NETWORK;

    m_need_aggre_domain = need_aggre_domain;
    m_need_rel_aggre_domain = need_rel_aggre_domain;
    bool bRet = Reload();
    assert(bRet);

    return bRet;

}

bool Domain::GetDataFromServer(MessageType msg,
                               std::list<string>* result)
{
    result->clear();

    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == fd)
    {
        WRITE_LOG("create socket failed.");
        return false;
    }

    if (!InitSocket(fd))
    {
        close(fd);
        return false;
    }

    // ���� server
    struct sockaddr_in stAddr;
    memset(&stAddr, 0, sizeof(stAddr));
    stAddr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    stAddr.sin_family = AF_INET;
    stAddr.sin_port = htons(m_port);

    if (stAddr.sin_addr.s_addr == INADDR_NONE)
    {
        WRITE_LOG("invalid socket address.");
        close(fd);
        return false;
    }

    if (connect(fd, (struct sockaddr*)&stAddr, sizeof(sockaddr_in)) < 0)
    {
        stringstream ss;
        ss << "connect to " << m_ip << ":" << m_port << " failed.";

        WRITE_LOG(ss.str());
        close(fd);
        return false;
    }

    SpiderPkg pkg = {};
    pkg.iBodyLen = htonl(4);
    pkg.dwMsgType = msg;

    size_t size = sizeof(SpiderPkg);

    int send = SendTcpData(fd, (char*)&pkg, size);
    if (send != (int)size)
    {
        WRITE_LOG("send spiderpkg failed.");
        close(fd);
        return false;
    }

    unsigned int arr[2];
    int read = RecvTcpData(fd, (char*)&arr[0], 8);
    if (read != 8)
    {
        stringstream ss;
        ss << "recv msg type, date length failed. expect 8 but received " << read;
        WRITE_LOG(ss.str());

        close(fd);
        return false;
    }

    unsigned int data_length = arr[1];

    char* buffer = new char[data_length];
    read = RecvTcpData(fd, buffer, data_length);

    stringstream ss;
    ss << "data length: " << data_length << ", received length: " << read;
    WRITE_LOG(ss.str());

    if (read != (int)data_length)
    {
        delete [] buffer;
        close(fd);
        return false;
    }

    close(fd);

    unsigned int start = 0;
    for (unsigned int i = 0; i < data_length; i++)
    {
        if (buffer[i] == '\n')
        {
            std::string str(buffer + start, i - start);
            start = i + 1;

            if (!str.empty())
            {
                result->push_back(str);
            }
        }
    }

    delete [] buffer;
    return true;
}

bool Domain::InitSocket(int fd)
{
    // ���� socket Ϊ����ģʽ
    int arg = fcntl(fd, F_GETFL, 0); 
    if (fcntl(fd, F_SETFL, arg& ~O_NONBLOCK) == -1)
    {
        return false;
    }

    // ���ý��շ��ͳ�ʱʱ��Ϊ 10 ��
    int timeout = 10;
    struct timeval tv = {timeout, 0};
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)) < 0)
    {
        return false;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv)) < 0)
    {
        return false;
    }

    return true;
}

int Domain::RecvTcpData(int fd, char* buffer, size_t size)
{
    char* p = buffer;
    size_t read = 0;
    int last_read = 0;
    while (read < size)
    {
        do
        {
            last_read = recv(fd, p, size - read, 0);
        }while ((last_read < 0) && (errno == EINTR));

        if (last_read > 0)
        {
            read += last_read;
            p += last_read;
        }
        else
        {
            return read;
        }
    }

    return read;
}

int Domain::SendTcpData(int fd, char* buffer, size_t size)
{
    char* p = buffer;
    size_t sent = 0;
    int last_send = 0;
    while (sent < size)
    {
        do
        {
            last_send = send(fd, p, size - sent, 0);
        }while ((last_send < 0) && (errno == EINTR));

        if (last_send > 0)
        {
            sent += last_send;
            p += last_send;
        }
        else
        {
            return sent;
        }
    }

    return sent;
}

bool Domain::Reload()
{
    WRITE_LOG("begin reload data ... ");

    // ������Ǵ�������أ���ֱ�ӷ���
    if (m_data_load_method != FROM_NETWORK)
    {
        return true;
    }

    bool ret = false;
    list<string> result;

    ret = GetDataFromServer(GET_DOMAIN_SUFFIX, &result);
    if (!ret)
    {
        return false;
    }
    else
    {
        SetDomainSuffix(result);
    }

    if (m_need_aggre_domain)
    {
        ret = GetDataFromServer(GET_AGGRE_DOMAIN, &result);
        if (!ret)
        {
            return false;
        }

        SetAggreDomainSuffix(result);
    }

    if (m_need_rel_aggre_domain)
    {
        ret = GetDataFromServer(GET_REL_AGGRE_DOMAIN, &result);
        if (!ret)
        {
            return false;
        }
        
        SetRelAggreDomainSuffix(result);
    }
    
    WRITE_LOG("reload data finished. ");

    return true;
}

void Domain::SetDomainSuffix(const list<string>& data)
{
    AutoLocker locker(&m_mutex);

    m_domain_suffix.clear();
    m_domain_suffix.resize(MAX_HASH_SIZE);

    list<string>::const_iterator p;
    for (p = data.begin(); p != data.end(); p++)
    {
        const string& str = *p;
        unsigned int hash = GetHash(str.c_str(), str.length());
        m_domain_suffix[hash].push_back(str.c_str());
    }
}

void Domain::SetAggreDomainSuffix(const list<string>& data)
{
    AutoLocker locker(&m_mutex);

    m_not_aggre_subdomain.clear();
    m_aggre_domain_suffix.clear();
    
    m_not_aggre_subdomain.resize(MAX_HASH_SIZE);
    m_aggre_domain_suffix.resize(MAX_HASH_SIZE);
    
    list<string>::const_iterator p;
    for (p = data.begin(); p != data.end(); p++)
    {
        const string& str = *p;
        string::size_type pos = str.find('\t');
        
        string suffix;
        string sign;
        int n = 0;

        if (pos == string::npos)
        {
            suffix = str;
        }
        else
        {
            suffix = str.substr(0, pos);
            sign = str.substr(pos + 1, str.length() - pos);
            n = atoi(sign.c_str());
        }

        unsigned int hash = GetHash(suffix.c_str(), suffix.length());

        if (0 == n)
        {
            m_not_aggre_subdomain[hash].push_back(suffix);
        }
        else
        {
            m_aggre_domain_suffix[hash].push_back(suffix);
        }
    }
}

void Domain::SetRelAggreDomainSuffix(const list<string>& data)
{
    AutoLocker locker(&m_mutex);

    m_rel_not_aggre_subdomain.clear();
    m_rel_aggre_domain_suffix.clear();
    
    m_rel_not_aggre_subdomain.resize(MAX_HASH_SIZE);
    m_rel_aggre_domain_suffix.resize(MAX_HASH_SIZE);
    
    list<string>::const_iterator p;
    for (p = data.begin(); p != data.end(); p++)
    {
        const string& str = *p;
        string::size_type pos = str.find('\t');
        
        string suffix;
        string sign;
        int n = 0;

        if (pos == string::npos)
        {
            suffix = str;
        }
        else
        {
            suffix = str.substr(0, pos);
            sign = str.substr(pos + 1, str.length() - pos);
            n = atoi(sign.c_str());
        }

        unsigned int hash = GetHash(suffix.c_str(), suffix.length());

        if (0 == n)
        {
            m_rel_not_aggre_subdomain[hash].push_back(suffix);
        }
        else
        {
            m_rel_aggre_domain_suffix[hash].push_back(suffix);
        }
    }
}

bool Domain::LoadDataFromFile(const char* file_name, list<string>* data)
{
    data->clear();
    ifstream ifs(file_name);
    if (!ifs)
    {
        stringstream ss;
        ss << "open file " << file_name << " failed.";
        WRITE_LOG(ss.str());
        return false;
    }

    string line;
    while (getline(ifs, line))
    {
        if (!line.empty())
        {
            data->push_back(line);
        }
    }

    stringstream ss;
    ss << "total load " << data->size() << " lines from file " << file_name;
    WRITE_LOG(ss.str());
    
        return true;
}

string Domain::GetCurTime()
{
    struct tm	*lptmNow;
    time_t		nTime, nPreciseTime;
    char		sTime[256];

    struct timeval	tvNow;

    gettimeofday(&tvNow, NULL);
    nTime = tvNow.tv_sec;
    nPreciseTime = tvNow.tv_usec;

    memset(sTime, 0, sizeof(sTime));
    struct tm	tmNow;
    lptmNow = localtime_r((const time_t *)&nTime, &tmNow);
    sprintf(sTime, "%04d-%02d-%02d %02d:%02d:%02d:%06ld",
        1900+lptmNow->tm_year, lptmNow->tm_mon+1, lptmNow->tm_mday,
        lptmNow->tm_hour, lptmNow->tm_min, lptmNow->tm_sec, nPreciseTime);

    return string(sTime);
}

void Domain::WriteLog(const string& strFile, unsigned int dwLine, const string& strLog)
{
    string strTime = GetCurTime();
    string strDate = strTime.substr(0, 10);

    string strFileName = "./" + strDate + "_load.log";
    ofstream oFile(strFileName.c_str(), ios_base::app);
    if (oFile)
    {
        oFile << endl
            << "time: " << strTime << endl
            << "file: " << strFile << endl
            << "line: " << dwLine << endl
            << strLog << endl;
    }

    oFile.flush();
    oFile.close();
}

void Domain::GetReverseSubDomain(char* dest, const char* src)
{
    if (NULL == dest || NULL == src)
    {
        return;
    }

    int len = strlen(src);
    GetReverseSubDomain(dest, src, len);
}
void Domain::GetReverseSubDomain(char* dest, const char* src, int len)
{
    if (NULL == dest || NULL == src || 0 == len)
    {
        return;
    }

    dest[len] = 0;
    const char* check_end = src + len - 1;

    // �ȿ���û�ж˿�
    const char* port = strchr(src, ':');
    
    // �� dest �����дλ��
    int write_pos = 0;

    // ����ж˿�
    if (port != NULL)
    {
        // �����˿��Ҳ�����ݣ�����:����
        int copy_len = check_end - port + 1;
        memcpy(dest + (port - src), port, copy_len);
        
        check_end = port - 1;
    }

    // ���һ�μ����ɵ�
    const char* last_check_end = check_end;

    const char* p = NULL;
    for (p = check_end; p > src; p--)
    {
        if (*p == '.')
        {
            memcpy(dest + write_pos, p + 1, last_check_end - p);
            write_pos += last_check_end - p;
            *(dest + write_pos) = '.';
            write_pos++;
            last_check_end = p - 1;
        }

    }
    memcpy(dest + write_pos, p, last_check_end - p + 1);
}

#ifdef USE_MD5
unsigned long long Domain::GetSubdomainID() const
{
    unsigned long long subdomain_id = 0;
    if (m_suffix_pos != -1)
    {
        MD5::Hash64(m_subdomain, subdomain_id);
    }

    return subdomain_id;
}

unsigned long long Domain::GetDomainID() const
{
    unsigned long long domain_id = 0;
    if (m_suffix_pos != -1)
    {
        MD5::Hash64(m_domain, domain_id);
    }

    return domain_id;
}
#endif