/********************************
 FileName: executor/executor.cpp
 Author:   WangMin
 Date:     2013-08-14
 Version:  0.1
 Description: executor main
*********************************/

#include <iostream>
#include <sys/stat.h>
#include <sys/wait.h>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <gflags/gflags.h>

#include "common/clynn/rpc.h"
#include "executor/service.h"
#include "executor/dispatcher.h"
#include "executor/resource_manager.h"

using std::string;
using std::cout;
using std::endl;
using std::auto_ptr;

using log4cplus::Logger;
using log4cplus::ConsoleAppender;
using log4cplus::FileAppender;
using log4cplus::Appender;
using log4cplus::Layout;
using log4cplus::PatternLayout;
using log4cplus::helpers::SharedObjectPtr;

// gflag, config for executor
DECLARE_int32(port);
DECLARE_string(collector_endpoint);
DECLARE_string(scheduler_endpoint);
/*DECLARE_string(interface);
DECLARE_string(img_dir);*/
DECLARE_string(log_path);
DECLARE_string(libvirt_dir);
DECLARE_string(xml_template);
DECLARE_string(lxc_dir);
DECLARE_string(lxc_template);

extern void* TaskProcessor(void* unused);
extern void* VMProcessor(void* unused);
extern void* HeartbeatProcessor(void* unused);

// executor
int ExecutorEntity(int argc, char **argv) {
    // config file
    if (argc > 1)
        google::ParseCommandLineFlags(&argc, &argv, true);
    else
        google::ReadFromFlagsFile("../conf/executor.conf", argv[0], true);


    // initilize log log4cplus
    SharedObjectPtr<Appender> append(new FileAppender(FLAGS_log_path + "/executor.log"));
    append->setName(LOG4CPLUS_TEXT("append for executor"));
    auto_ptr<Layout> layout(new PatternLayout(LOG4CPLUS_TEXT("%d{%y/%m/%d %H:%M:%S} %p [%l]: %m %n")));
    append->setLayout(layout);
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("executor"));
    logger.addAppender(append);
    logger.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
    LOG4CPLUS_DEBUG(logger, "This is the FIRST debug message");
    LOG4CPLUS_INFO(logger, "This is the FIRST info message");
    LOG4CPLUS_ERROR(logger, "This is the FIRST error message");
  
    // check libvirt work dir
    if(access(FLAGS_libvirt_dir.c_str(), F_OK) == -1) {
       if(mkdir(FLAGS_libvirt_dir.c_str(),0755) != 0){
           LOG4CPLUS_ERROR(logger, "Can't create libvirt(kvm) work dir: " << FLAGS_libvirt_dir);
           exit(-1);
       }
    }

    // check lxc work dir
    if(access(FLAGS_lxc_dir.c_str(), F_OK) == -1) {
       if(mkdir(FLAGS_lxc_dir.c_str(),0755) != 0){
           LOG4CPLUS_ERROR(logger, "Can't create lxc work dir: " << FLAGS_lxc_dir);
           exit(-1);
       }
    }
   
    // TODO
    // get current dir
    char c_dir[100];
    if (getcwd(c_dir, sizeof(c_dir)-1) != NULL) {
        LOG4CPLUS_INFO(logger, "current dir:" << c_dir);
    } else {
        LOG4CPLUS_ERROR(logger, "Fails to get current dir.");
    }
    string separator = "/";
    FLAGS_xml_template = c_dir + separator + FLAGS_xml_template;
    FLAGS_lxc_template = c_dir + separator + FLAGS_lxc_template;

    // init resource_manager
    if (!ResourceMgrI::Instance()->Init()) {
        LOG4CPLUS_ERROR(logger, "Can't initialize resource manager.");
        exit(-1);
    }

    // work thread
    pthread_t task_t;
    pthread_create(&task_t, NULL, TaskProcessor, NULL);

    pthread_t vm_t;
    pthread_create(&vm_t, NULL, VMProcessor, NULL);

    // TODO
    pthread_t hb_t;
    pthread_create(&hb_t, NULL, HeartbeatProcessor, NULL);

    /* event dispatcher */
    // state event
    // Handler* state_event_handler = new Handler;
    // state_event_handler->Start();
    // EventDispatcherI::Instance()->Register(EventType::STATE_EVENT, state_event_handler);

    // action event
    Handler* action_event_handler = new Handler;
    action_event_handler->Start();
    EventDispatcherI::Instance()->Register(EventType::ACTION_EVENT, action_event_handler);

    // task event
    Handler* task_event_handler = new Handler;
    task_event_handler->Start();
    EventDispatcherI::Instance()->Register(EventType::TASK_EVENT, task_event_handler);

    /*TaskID id;
    id.job_id = 1;
    id.task_id = 1;
    EventPtr event(new KilledEvent(id, true));
    EventDispatcherI::Instance()->Dispatch(event->GetType())->PushBack(event);*/

    cout << "Executor is OK." << endl;

    // Listen for service 
    Rpc<ExecutorService, ExecutorProcessor>::Listen(FLAGS_port);
    //int port = 9999; 
    //Rpc<ExecutorService, ExecutorProcessor>::Listen(port);

    return 0;
}

int main(int argc, char **argv) {
    // is root?
    if (geteuid() != 0) {
        fprintf(stderr, "Executor: must be run as root, or sudo run it.\n");
        exit(1);
    }

    // monitor ExecutorEntity
    while(true) {
        int32_t status;
        int32_t pid = fork();
        if (pid != 0) {
            // parent process, start executorEntity when ExecutorEntity fail
            if (waitpid(pid, &status, 0) > 0) {
                continue;
            }
        } else {
            // child process
            ExecutorEntity(argc, argv);
        }
    }
    return 0; 
}
