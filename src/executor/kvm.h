/********************************
 FileName: executor/kvm.h
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: kvm, inherit from vm
*********************************/

#ifndef SRC_EXECUTOR_KVM_H
#define SRC_EXECUTOR_KVM_H

#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#include <gflags/gflags.h>
#include "executor/vm.h"

DECLARE_int32(vm_hb_interval);

class KVM : public VM {
public:
    explicit KVM(const TaskInfo& info) : VM(info) {
        m_domain_ptr = NULL;
        m_timestamp = -1;
        m_time_to_death = 6 * FLAGS_vm_hb_interval; 
    }

    ~KVM() {
        // free domain
        if(m_domain_ptr) {
            virDomainFree(m_domain_ptr);
        }
        // clear the work directory
        if (!m_dir.empty()) {
            string cmd = "rm -r " + m_dir;
            system(cmd.c_str());
        }
    }

    // virtual function, from VM
    int32_t CreateEnv(); // create kvm and install

    bool Execute();  // execute the task, run the app

    bool Stop();

    bool Kill();

    HbVMInfo GetHbVMInfo();    //get heartbeart

    void SetHbVMInfo(const VM_HbVMInfo& hb_vm_info);

    // unique in KVM 
    virDomainPtr GetDomainPtr() const;

    //void SetDomainPtr(virDomainPtr ptr);

    int32_t GetVNCPort() const;

    //void SetVNCPort(int32_t port);

    string GetVNet() const;

    void SetVNet(string vnet);

private:
    // virtual function, from VM
    void SetName();

    // Init Heartbeat
    void InitHeartbeat();    

    // Judge State
    VMState::type GetState(); 
 
    // set Member Variable 
    int32_t Init();

    // cp img, type = raw
    int32_t CopyImage();

    // clone img from template img, type = qcow2
    int32_t CloneImage();

    int32_t ConfigVirXML();
 
    int32_t CreateKVM();

    int32_t SetVNetByXML();

    int32_t InstallApp();

    int32_t StartApp();

    // get hb
    double GetCpuUsage();

    double GetMemoryUsage();

private:
    virDomainPtr m_domain_ptr;
    string m_dir;   // work dir
    string m_img;   // .img
    string m_iso;   // .iso
    string m_conf;  // CONF
    string m_xml;   // libvirt xml config content
    int32_t m_vnc_port;
    string m_vnet;
   
    // Heartbeat and state
    HbVMInfo m_hb_vm_info;
    // time_t m_start_time;
    int32_t m_timestamp;
    int32_t m_time_to_death;

    // static
    static string m_xml_template; // libvirt xml config template content
    static virConnectPtr m_conn;  // libvirt qemu connect

    // report resource, is first?
    // bool m_first;
    // double m_prev_cpu;
    // double m_prev_total;

};

#endif
