#include "collector/machine_pool.h"

using lynn::ReadLocker;
using lynn::WriteLocker;

void MachinePool::Insert(const MachinePtr& machine){
    string endpoint = machine->GetEndpoint();
    WriteLocker locker(m_lock);
    m_machine_pool[endpoint] = machine;
}
