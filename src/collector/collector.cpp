/********************************
 FileName: collector/collector.cpp
 Author:   ZhangZhang 
 Date:     2013-08-16
 Version:  0.1
 Description: collector main
*********************************/

#include <iostream>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <gflags/gflags.h>

#include "include/proxy.h"
#include "common/clynn/rpc.h"

#include "collector/config_manager.h"

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

int main(int argc, char **argv)
{
    if(argc > 1) {
        google::ParseCommandLineFlags(&argc, &argv, true);
    } else {
        google::ReadFromFlagsFile("../conf/collector.conf", argv[0], true);
    }
    SharedObjectPtr<Appender> append(new FileAppender("collector.log"));
    append->setName(LOG4CPLUS_TEXT("append for collector"));
    auto_ptr<Layout> layout(new PatternLayout(LOG4CPLUS_TEXT("%d{%m/%d/%y %H:%M:%S} %p %l:%m %n")));
    append->setLayout(layout);
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("collector"));
    logger.addAppender(append);
    logger.setLogLevel(log4cplus::INFO_LOG_LEVEL);
    LOG4CPLUS_INFO(logger, argv[0] << "daemon begin");

    COLLECTORCONFIG::Instance()->Init();

    return 0;
}
