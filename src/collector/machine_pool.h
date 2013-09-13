/********************************
 *  FileName: collector/machine_pool.h
 *   Author:   ZhangZhang
 *    Description: the machine pool class
 *    *********************************/

#ifndef SRC_COLLECTOR_MACHINE_POOL_H
#define SRC_COLLECTOR_MACHINE_POOL_H

#include <map>
#include "common/clynn/rwlock.h"
#include "common/clynn/singleton.h"

#include "collector/machine.h"

using std::string;
using std::map;
using clynn::RWLock;

class MachinePool {
public:
    explicit MachinePool(){};
    ~MachinePool() {};
    void Insert(const MachinePtr& machine_ptr);
private:
    RWLock m_lock;
    map<string, MachinePtr> m_machine_pool;
};

typedef Singleton<MachinePool> MachinePoolI;
#endif
