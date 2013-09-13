#include <fstream>
#include <iostream>
#include <map>
#include "xfs/baselib/svrpublib/server_publib.h"
using namespace xfs::base;

class ParseProtocolInl
{
public:
    void Parse(std::string file);
    std::string GetServiceType(int32_t type);
    std::string GetKey(int32_t type, int16_t key);
    void Debug();
private:
    std::map<int32_t, std::string> st;
    std::map<int32_t, std::map<uint16_t, std::string> > st_key;
};