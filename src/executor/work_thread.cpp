/********************************
 FileName: executor/work_thread.h
 Author:   WangMin
 Date:     2013-09-04
 Last Update: 
 Version:  0.1
 Description: work thread
*********************************/

#include <stdlib.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <gflags/gflags.h>

#include "include/proxy.h"
#include "common/clynn/rpc.h"

#include "task_entity_pool.h"
#include "vm_pool.h"
#include "resource_manager.h"

using log4cplus::Logger;

DECLARE_string(collector_endpoint);
DECLARE_int32(hb_interval);

static Logger logger = Logger::getInstance("executor");

// thread, start task
void* TaskProcessor(void* unused) {
    while (true) {
        // at one time, only start task that first found
        TaskPoolI::Instance()->StartTask();
        usleep(1000*100);
    }
    return NULL;
}

// thread, start vm
void* VMProcessor(void* unused) { 
    while (true) {
        // at one time, only start vm that first found
        VMPoolI::Instance()->StartVM();
        usleep(1000*100);
    }
    return NULL;
}

// thread, send heartbeat periodically
void* HeartbeatProcessor(void* unused) {
    while(true) {
       // HbMachineInfo info;
       // info = ResourceMgrI::Instance()->GetMachineInfo();
       string hb_str = ResourceMgrI::Instance()->GenerateHb();
       // TODO
       printf("send heart beat\n");
       try {
           Proxy<CollectorClient> proxy = Rpc<CollectorClient, CollectorClient>::GetProxy(FLAGS_collector_endpoint);
           proxy().SendHeartbeat(hb_str);
       } catch (TException &tx) {
           LOG4CPLUS_ERROR(logger, "send heartbeat error: " << tx.what());
       } 
       sleep(FLAGS_hb_interval);
    }
    return NULL;
}
