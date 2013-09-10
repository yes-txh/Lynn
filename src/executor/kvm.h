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
#include "executor/vm.h"

class KVM : public VM {
public:
    explicit KVM(const TaskInfo& info) : VM(info) {
        m_domain_ptr = NULL;
        m_first = true;
        m_prev_cpu = 0.0;
        m_prev_total = 0.0;
    }

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
    virDomainPtr GetDomainPtr();

    //void SetDomainPtr(virDomainPtr ptr);

    int32_t GetVNCPort();

    //void SetVNCPort(int32_t port);

    string GetVNet();

    void SetVNet(string vnet);

private:
    // virtual function, from VM
    void SetName();
  
    int32_t Init();

    // cp img, type = raw
    int32_t CopyImage();

    // clone img from template img, type = qcow2
    int32_t CloneImage();

    int32_t ConfigVirXML();
 
    int32_t CreateVM();

    int32_t SetVNetByXML();

    int32_t Install();

    // get hb
    double GetCpuUsage();

    double GetMemoryUsage();

private:
    virDomainPtr m_domain_ptr;
    string m_dir;   // work dir
    string m_img;   // .img
    string m_iso;   // .iso
    string m_conf;  // CONF
    string m_xml;   // libvirt xml config
    int32_t m_vnc_port;
    string m_vnet;
    static string m_xml_template;
    static virConnectPtr m_conn;

    // report resource, is first?
    bool m_first;
    double m_prev_cpu;
    double m_prev_total;
};

#endif
