#ifndef COMMOM_SYSTEM_NET_GET_IP_HPP
#define COMMOM_SYSTEM_NET_GET_IP_HPP

#include <stdlib.h>

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#endif

#include <vector>
#include "common/system/net/socket.hpp"

#ifdef _WIN32

inline bool GetLocalIpList(std::vector<IPAddress>* v)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return false;
    }

    char hostname[128];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        struct hostent* host = gethostbyname(hostname);
        if (host)
        {
            int i = -1;
            do
            {
                i++;
                v->push_back(IPAddress(*reinterpret_cast<IN_ADDR*>
                            (host->h_addr_list[i])));
            } while (host->h_addr_list[i] + host->h_length < host->h_name);
        }
    }
    WSACleanup();
    return true;
}
#pragma comment(lib, "ws2_32.lib")

#else

inline bool GetLocalIpList(std::vector<IPAddress>* v)
{
    v->clear();

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        return false;
    }

    // 初始化ifconf
    struct ifconf ifconf;
    char buffer[512];
    ifconf.ifc_len = 512;
    ifconf.ifc_buf = buffer;

    // 获取所有接口信息
    ioctl(sockfd, SIOCGIFCONF, &ifconf);
    close(sockfd);

    struct ifreq *ifreq = reinterpret_cast<struct ifreq*>(buffer);
    for (size_t i = 0; i < (ifconf.ifc_len / sizeof(struct ifreq)); ++i)
    {
        IPAddress ip(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
        if (strcmp(ifreq->ifr_name, "lo") != 0 && !ip.IsLoopback())
        {
            v->push_back(ip);
        }
        ifreq++;
    }
    return true;
}

#endif

#endif // COMMOM_SYSTEM_NET_GET_IP_HPP
