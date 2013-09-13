/********************************
 FileName: executor/work_thread.h
 Author:   WangMin
 Date:     2013-09-04
 Last Update: 
 Version:  0.1
 Description: work thread
*********************************/

#include <stdlib.h>

#include "include/proxy.h"
#include "common/clynn/rpc.h"
#include "task_entity_pool.h"
#include "vm_pool.h"

void* TaskProcessor(void* unused)
{
    while (true)
    {
        // at one time, only start task that first found
        TaskPoolI::Instance()->StartTaskEntity();
        usleep(1000*100);
    }
    return NULL;
}

void* VMProcessor(void* unused)
{
    while (true)
    {
        // at one time, only start vm that first found
        VMPoolI::Instance()->StartVM();
        
        usleep(1000*1000);
    }
    return NULL;
}

