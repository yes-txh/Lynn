#include "collector/machine_pool.h"

using clynn::ReadLocker;
using clynn::WriteLocker;

void MachinePool::Insert(const MachinePtr& machine){
    string endpoint = machine->GetEndpoint();
    WriteLocker locker(m_lock);
    m_machine_pool[endpoint] = machine;
}
