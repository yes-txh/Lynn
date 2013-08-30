/********************************
 *  FileName: collector/machine.h
 *   Author:   ZhangZhang
 *    Description: the machine class
 *    *********************************/

#ifndef SRC_COLLECTOR_MACHINE_H
#define SRC_COLLECTOR_MACHINE_H

#include <string>
#include <boost/shared_ptr.hpp>
#include <classad/classad.h>

using std::string;
using boost::shared_ptr;

class Machine {
public:
    explicit Machine(const string& machine_ad);
    string GetEndpoint() {
         return m_endpoint;
    }
private:
    string m_endpoint;
};

typedef shared_ptr<Machine> MachinePtr;

#endif
