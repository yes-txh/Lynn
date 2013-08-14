/********************************
 FileName: scheduler/scheduler.cpp
 Author:   
 Date:     2013-08-14
 Version:  0.1
 Description: scheduler main
*********************************/

#include <iostream>
#include <glog/logging.h>

#include "include/proxy.h"
#include "common/rpc.h"

using std::string;
using std::cout;
using std::endl;

int main(int argc, char **argv)
{
    /*// CELLOS_HOME
    if (!getenv("CELLOS_HOME"))
    {
        fprintf(stderr, "environment value CELLOS_HOME is not set.\n");
        return -1;
    }
    string cellos_home = string(getenv("CELLOS_HOME"));
    if (!cellos_home.c_str()) {
        fprintf(stderr, "environment value CELLOS_HOME is not set.\n");
        return -1;
    }
    printf("CELLOS_HOME:%s\n",cellos_home.c_str());
   
    // initilize log
    google::InitGoogleLogging(argv[0]);
    string info_log = cellos_home + "/log/slave_info_";
    google::SetLogDestination(google::INFO, info_log.c_str());
    string warning_log = cellos_home + "/log/slave_warning_";
    google::SetLogDestination(google::WARNING, warning_log.c_str());
    string error_log = cellos_home + "log/slave_error_";
    google::SetLogDestination(google::ERROR, error_log.c_str());
    string fatal_log = cellos_home + "/log/slave_fatal_";
    google::SetLogDestination(google::FATAL, fatal_log.c_str());*/

    // initilize log
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, "../log/executor_info_");
    google::SetLogDestination(google::WARNING, "../log/executor_warning_");
    google::SetLogDestination(google::ERROR, "../log/executor_error_");
    google::SetLogDestination(google::FATAL, "../log/executor_fatal_");

    cout << "Scheduler is OK." << endl;
    // Listen for service 
    //Rpc<ExecutorService, ExecutorProcessor>::Listen(
    //    atoi(ConfigI::Instance()->Get("port").c_str()));
    const string executor_endpoint = "127.0.0.1:9997";
    const int TIME_OUT = 2000;
    Proxy<ExecutorClient> proxy = Rpc<ExecutorClient, ExecutorClient>::GetProxy(
           executor_endpoint , TIME_OUT);
    proxy().Helloworld();

    return 0;
}
