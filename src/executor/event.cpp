/********************************
 FileName: executor/event.cpp
 Author:   WangMin
 Date:     2013-09-24
 Version:  0.1
 Description: event, and its handler
*********************************/

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <gflags/gflags.h>

#include "executor/event.h"

using log4cplus::Logger;

static Logger logger = Logger::getInstance("executor");

// task is killed
bool KilledEvent::Handle() {
    printf("KilledEvent\n");
    return true;
}

// kill task
bool KillActionEvent::Handle() {
    printf("KillActionEvent\n");
    return true;
}
