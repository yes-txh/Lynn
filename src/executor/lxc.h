/********************************
 FileName: executor/lxc.h
 Author:   WangMin
 Date:     2013-09-17
 Version:  0.1
 Description: lxc(Linux Container), inherit from vm
*********************************/

#ifndef SRC_EXECUTOR_LXC_H
#define SRC_EXECUTOR_LXC_H

#include "sys/types.h"
#include "executor/vm.h"

class LXC : public VM {
public:
    explicit LXC(const TaskInfo& info) : VM(info) {
        m_first = true;
        m_prev_cpu = 0.0;
        m_prev_total = 0.0;
    }

    ~LXC() {
        // clear the work directory
        if (!m_dir.empty()) {
            string cmd = "rm -r " + m_dir;
            system(cmd.c_str());
        }    
    }

    // virtual function, from VM
    int32_t CreateEnv(); // create LXC

    // TODO
    int32_t InstallApp() { return 0; }

    // @brief: fork process and run the task
    bool Execute();

    bool Stop();

    bool Kill();

    HbVMInfo GetHbVMInfo();    //get heartbeart

    void SetHbVMInfo(const VM_HbVMInfo& hb_vm_info);

    pid_t GetPid();

private:
    // virtual function, from VM
    void SetName();

    // set Member Variable 
    int32_t Init();

    int32_t CreateLXC();

    // @brief: set ip addr with conf
    int32_t SetIPConf();

    // @brief: close all the fd inherited from parent according to /proc/pid/fd
    int32_t CloseInheritedFD();

    // @brief: re-direct log to work directory
    int32_t RedirectLog();
    
    // @brief: set container resource limit
    void SetResourceLimit(); 

private:
    pid_t m_pid;    // 进程号
    string m_dir;   // work dir
    string m_conf_bak;  // lxc.conf path
    string m_conf_path; // lxc.conf path
    HbVMInfo m_hb_vm_info;

    static string m_conf_template;

    // report resource, is first?
    bool m_first;
    double m_prev_cpu;
    double m_prev_total;
};

#endif


