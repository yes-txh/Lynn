/********************************
 FileName: executor/lxc.cpp
 Author:   WangMin
 Date:     2013-09-17
 Version:  0.1
 Description: lxc(Linux Container), inherit from vm
*********************************/

#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
//#include <sys/unistd.h>
//#include <dirent.h>
#include <fcntl.h> // for func open
//#include <time.h>
//#include <signal.h>


#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <gflags/gflags.h>

#include "lxc/lxc.h"
#include "lxc/conf.h"

#include "executor/system.h"
#include "executor/lxc.h"
#include "executor/task_entity_pool.h"
#include "executor/resource_manager.h"

using std::cout;
using std::endl;
using std::stringstream;
using std::ifstream;
using std::ofstream;
using log4cplus::Logger;
using clynn::WriteLocker;
using clynn::ReadLocker;

DECLARE_string(lxc_dir);
DECLARE_string(lxc_template);

static Logger logger = Logger::getInstance("executor");

string LXC::m_conf_template = "";

/// @brief: public function
// virtual function, from VM
// virtual CreateEnv, include ..
int32_t LXC::CreateEnv() {
    if (Init() != 0) {
        LOG4CPLUS_ERROR(logger, "Fails to init lxc, name:" << GetName() << ", job_id:" << GetID().job_id << ", task_id:" << GetID().task_id);
        return -1;
    }
    return 0;
}

// execute the task, run the app
bool LXC::Execute() {
    if (CreateLXC() != 0) {
        LOG4CPLUS_ERROR(logger, "Fails to execute lxc, name:" << GetName() << ", job_id:" << GetID().job_id << ", task_id:" << GetID().task_id);
        return false;
    }
    return true;
}

bool LXC::Stop() {
    return true;
}

bool LXC::Kill() {
    return true;
}

HbVMInfo LXC::GetHbVMInfo() {
    // TODO
    HbVMInfo info;
    return info;
}

/// @brief: unique in KVM
pid_t LXC::GetPid() {
    return m_pid;
}

/// @brief: private function
// set name
void LXC::SetName() {
    // app_name + "_lxc_" + job_id + "_" + task_id
    TaskID id = GetID();
    stringstream ss_job, ss_task;
    ss_job << id.job_id;
    ss_task << id.task_id;
    string name = GetTaskInfo().app_info.name + "_lxc_" + ss_job.str() + "_" + ss_task.str();
    SetNameByString(name);
}

// set name m_dir, mk work dir, copy lxc.conf, and change work director to m_dir
int32_t LXC::Init() {
    // set name, img, iso
    SetName();
    m_dir = FLAGS_lxc_dir + "/" + GetName() + "/";
    m_conf_bak = m_dir + "lxc_bak.conf";
    m_conf_path = m_dir + "lxc.conf";

    if ("" == m_conf_template) {
        m_conf_template = FLAGS_lxc_template;
    }

    TaskPtr task_ptr = GetTaskPtr(); 

    // check total work directory
    if (chdir(FLAGS_lxc_dir.c_str()) < 0) {
        LOG4CPLUS_ERROR(logger, "No lxc work directory:" << FLAGS_lxc_dir);
        task_ptr->TaskFailed();
        return -1;
    }

    // mkdir work dir
    if (access(m_dir.c_str(), F_OK) == -1) {
        if (mkdir(m_dir.c_str(), 0755) != 0) {
           LOG4CPLUS_ERROR(logger, "Can't create lxc work dir:" << m_dir);
           task_ptr->TaskFailed();
           return -1;
        }
    }

    // cp default lxc.conf to m_dir
    string cmd1 = "cp " + m_conf_template + " " + m_conf_bak;
    int32_t ret1 = system(cmd1.c_str());
    ret1 = ret1 >> 8;
    if (ret1 != 0) {
        LOG4CPLUS_ERROR(logger, "Can't copy default lxc.conf for " << m_conf_bak);
        LOG4CPLUS_ERROR(logger, "cmd1:" << cmd1);
        task_ptr->TaskFailed();
        return -1;
    }

    string cmd2 = "cp " + m_conf_template + " " + m_conf_path;
    int32_t ret2 = system(cmd2.c_str());
    ret2 = ret2 >> 8;
    if (ret1 != 0) {
        LOG4CPLUS_ERROR(logger, "Can't copy default lxc.conf for " << m_conf_path);
        LOG4CPLUS_ERROR(logger, "cmd2:" << cmd2);
        task_ptr->TaskFailed();
        return -1;
    }
 
    // change work directory
    if (chdir(m_dir.c_str()) < 0) {
        LOG4CPLUS_ERROR(logger, "Can't change work directory into " << m_dir);
        task_ptr->TaskFailed();
        return -1;
    }

    task_ptr->SetStates(TaskEntityState::TASKENTITY_STARTING, 50.0);

    return 0;
}

int32_t LXC::CreateLXC() {
    // fork child process
    m_pid = fork();
    if (0 == m_pid) {
        // child pid
        // close all the fd inherited from parent
        CloseInheritedFD();
        // TODO log
        RedirectLog();

        // TODO how config ip addr with api
        /***** lxc API *****
        // lxc_conf* conf = lxc_conf_init();
        // lxc_start(GetName().c_str(), 
        //          GetTaskInfo().app_info.exe_path.c_str(), conf);
        // free(conf);
        *******************/
        if (GetTaskInfo().vm_info.ip != "") {
            SetIPConf();
        } 

        // lxc-create
        string cmd1 = "lxc-create -n " + GetName() + " -f " + m_conf_path;
        int32_t ret1 = system(cmd1.c_str());
        ret1 = ret1 >> 8;
        if (ret1 != 0) {
            LOG4CPLUS_ERROR(logger, "Can't create lxc, cmd:" << cmd1);
            return -1;
        }

        // lxc-execute
        string cmd2 = "lxc-execute -n " + GetName() + " "
                    + GetTaskInfo().app_info.exe_path;
        int32_t ret2 = system(cmd2.c_str());
        ret2 = ret2 >> 8;
        if (ret2 != 0) {
            LOG4CPLUS_ERROR(logger, "Can't execute lxc, cmd:" << cmd2);
            return -1;
        }

        exit(-1);
    } else {
        // parent process
        sleep(1);
        TaskPtr task_ptr = GetTaskPtr();
        task_ptr->TaskStarted();
    }
    return 0;
} 

// Set IP Conf with lxc.conf
int32_t LXC::SetIPConf() {
    ifstream in(m_conf_bak.c_str());
    ofstream out(m_conf_path.c_str());
    string tmp;
    while (getline(in, tmp, '\n')) {
        int index_ipv4 = tmp.find("ipv4");
        if (index_ipv4 > 0 && index_ipv4 < 500)
        {
            out << tmp.substr(tmp.find('#') + 2, tmp.find('='));
            out << GetTaskInfo().vm_info.ip << endl;
            /*LOG(ERROR) << tmp.substr(tmp.find('#') + 2, tmp.find('='));
            LOG(ERROR) << m_info.lxc_ip;*/
        }
        else
            out << tmp.substr(tmp.find('#') + 2, strlen(tmp.c_str()) - 2)
                << endl;
            // LOG(ERROR) << tmp.substr(tmp.find('#') + 2, strlen(tmp.c_str()) - 2);
    }
    in.close();
    out.close();
    return 0;
}

// close inherited(ji cheng)
// must in child process
int32_t LXC::CloseInheritedFD() {
    // get proc path
    stringstream ss;
    string pid;
    ss << getpid();
    ss >> pid;
    string path = "/proc/" + pid + "/fd";
    // c style
    // char path[30] = {0};
    // snprintf(path, sizeof(path), "/proc/%d/fd", getpid());

    DIR* dp = opendir(path.c_str());
    if (!dp) {
       // open error 
       LOG4CPLUS_ERROR(logger, "Fails to open proc path:" << path <<
                       ", when close inherited FD");
       return -1;
    }

    dirent* ep = NULL;
    while ((ep = readdir(dp))) {
        int fd = atoi(ep->d_name);
        // get rid of stdin, stdout, stderr which is 0,1,2
        if (fd > 2)
            close(fd);
    }

    closedir(dp);
    return 0;
}

// redirect log
int32_t LXC::RedirectLog() {
    // create log name and path
    char c_timestamp[30] = {0};
    System::GetCurrentTime(c_timestamp, sizeof(c_timestamp));
    string timestamp = c_timestamp;

    string log_name = GetName() + "_" + timestamp;
    string log_path = m_dir + log_name;
    
    // open the log file redirect stdout and stderr
    int fd = open(log_path.c_str(), (O_RDWR | O_CREAT), 0644);
    // LOG(ERROR) << "fd:" << fd;
    // stdout->fd
    dup2(fd, 1);
    // stderr->stdout
    dup2(fd, 2);
    // dup2(1, 2);
    close(fd);
}
