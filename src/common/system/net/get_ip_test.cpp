#include <iostream>
#include <vector>
#include <string>
#include <gtest/gtest.h>
#include "common/system/net/get_ip.hpp"

using namespace std;

TEST(Net, GetLocalIp)
{
    vector<IPAddress> ips;
    GetLocalIpList(&ips);
    for (size_t i = 0; i < ips.size(); i++)
    {
        cout << ips[i].ToString() << endl;
    }
}
