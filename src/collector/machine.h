/********************************
 *  FileName: collector/machine.h
 *  Author:   ZhangZhang
 *  Description: the machine class
 *
 *********************************/

#ifndef SRC_COLLECTOR_MACHINE_H
#define SRC_COLLECTOR_MACHINE_H

#include <string>
#include <boost/shared_ptr.hpp>
#include <classad/classad.h>

using std::string;
using boost::shared_ptr;

class Machine {
public:
    Machine(){};
    Machine(const string& machine_ad){};
    int32_t ParseAttr(const string& machine_ad);
    string GetEndpoint();
private:
    string m_ip;
    int32_t m_port;
};

typedef shared_ptr<Machine> MachinePtr;

#endif
