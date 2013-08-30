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
#include "executor/vm.h"

class KVM : public VM {
public:
    explicit KVM(const TaskInfo& info) : VM(info) {}

    ~KVM() {
        if(m_domain_ptr) {
            virDomainFree(m_domain_ptr);
        }
    }

    // virtual function, from VM
    int32_t CreateEnv(); // create kvm

    bool Execute();  // execute the task, run the app

    bool Stop();

    bool Kill();

    HbVMInfo GetHbVMInfo();    //get heartbeart

    // unique in KVM 
    virDomainPtr GetDomainPtr() {
        return m_domain_ptr;
    }

    void SetDomainPtr(virDomainPtr ptr) {
        m_domain_ptr = ptr;
    }

    int32_t GetVNCPort() {
        return m_vnc_port;
    }

    void SetVNCPort(int32_t port) {
        m_vnc_port = port;
    }

    string GetVNet() {
        return m_vnet;
    }

    void SetVNet(string vnet) {
        m_vnet = vnet;
    }

private:
    // virtual function, from VM
    void SetName();
  
    int32_t Init();

    // cp img, type = raw
    int32_t CopyImage();

    // clone img from template img, type = qcow2
    int32_t CloneImage();
 
    int32_t CreateVM();

    int32_t Install();

private:
    virDomainPtr m_domain_ptr;
    string m_img;   // .img
    string m_iso;   // .iso
    int32_t m_vnc_port;
    string m_vnet; 
};

#endif
