/********************************
 FileName: executor/executor.cpp
 Author:   WangMin
 Date:     2013-08-14
 Version:  0.1
 Description: executor main
*********************************/

#include <iostream>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <gflags/gflags.h>

#include "common/rpc.h"
#include "executor/executor_service.h"

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
DECLARE_int32(heartbeat_interval);
DECLARE_string(interface);
DECLARE_string(log_path);

int main(int argc, char **argv) {
    // is root?
    if (geteuid() != 0) {
        fprintf(stderr, "Executor: must be run as root, or sudo run it.\n");
        exit(1);
    }

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

   
    cout << "Executor is OK." << endl;
    // Listen for service 
    //Rpc<ExecutorService, ExecutorProcessor>::Listen(
    //    atoi(ConfigI::Instance()->Get("port").c_str()));
    //int port = 9997; 
    //Rpc<ExecutorService, ExecutorProcessor>::Listen(port);

    return 0;
}
