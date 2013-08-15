/********************************
 FileName: executor/executor.cpp
 Author:   WangMin
 Date:     2013-08-14
 Version:  0.1
 Description: executor main
*********************************/

#include <iostream>

#include "common/rpc.h"
#include "executor/executor_service.h"

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
    //google::InitGoogleLogging(argv[0]);
    //google::SetLogDestination(google::INFO, "../log/executor_info_");
    //google::SetLogDestination(google::WARNING, "../log/executor_warning_");
    //google::SetLogDestination(google::ERROR, "../log/executor_error_");
    //google::SetLogDestination(google::FATAL, "../log/executor_fatal_");

    /*// is root?
    if (geteuid() != 0) 
    {  
        fprintf(stderr, "Executor must run as root, or sudo run it.\n");  
        exit(1);  
    } */
   
    cout << "Executor is OK." << endl;
    // Listen for service 
    //Rpc<ExecutorService, ExecutorProcessor>::Listen(
    //    atoi(ConfigI::Instance()->Get("port").c_str()));
    //int port = 9997; 
    //Rpc<ExecutorService, ExecutorProcessor>::Listen(port);

    return 0;
}
