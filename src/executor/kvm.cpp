/********************************
 FileName: executor/kvm.cpp
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: kvm, inherit from vm
*********************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include "common/rapidxml/rapidxml.hpp"
#include "common/rapidxml/rapidxml_utils.hpp"
#include "common/rapidxml/rapidxml_print.hpp"

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <gflags/gflags.h>

#include "executor/kvm.h"
#include "executor/task_entity_pool.h"
#include "executor/resource_manager.h"

using std::cout;
using std::stringstream;
using std::ifstream;
using std::ofstream;
using std::endl;
using namespace rapidxml;
using log4cplus::Logger;
using lynn::WriteLocker;
using lynn::ReadLocker;

// gflag 
// libvirt_dir = /var/lib/libvirt/images/
// kvm_xml = ../conf/kvm.xml
DECLARE_string(libvirt_dir);
DECLARE_string(xml_template);
DECLARE_int32(vm_hb_interval);

static Logger logger = Logger::getInstance("executor");

string KVM::m_xml_template = "";
virConnectPtr KVM::m_conn = NULL;


/// @brief: public function
// KVM & ~KVM are inplement in kvm.h

/// virtual function, from V
// virtual CreateEnv, include CreateVM and Install 
int32_t KVM::CreateEnv() {
    if (CreateVM() != 0) {
        LOG4CPLUS_ERROR(logger, "fails to create kvm");        
        return -1;
    }

    if (Install() != 0) {
        LOG4CPLUS_ERROR(logger, "fails to install app into kvm");                        
        return -1;
    }

    cout << "CreateEnv end" << endl;
    return 0;
}

// execute the task, run the app
bool KVM::Execute() {
    return true;
}

bool KVM::Stop() {
    return true;
}

bool KVM::Kill() {
    return true;
}

HbVMInfo KVM::GetHbVMInfo() {
}

/// unique in KVM 
virDomainPtr KVM::GetDomainPtr() {
    return m_domain_ptr;
}

//void KVM::SetDomainPtr(virDomainPtr ptr) {
//    m_domain_ptr = ptr;
//}

int32_t KVM::GetVNCPort() {
    return m_vnc_port;
}

//void KVM::SetVNCPort(int32_t port) {
//    m_vnc_port = port;
//}

string KVM::GetVNet() {
    return m_vnet;
}

void KVM::SetVNet(string vnet) {
    m_vnet = vnet;
}


/// @brief: private function
// set name
void KVM::SetName() {
    // app_name + "_kvm_" + task_id
    stringstream ss;
    ss << GetId();
    string name = GetTaskInfo().app_info.name + "_kvm_" + ss.str();
    SetNameByString(name);
}

// set name img iso, mk work dir, build libvirt connection, read xml_template into m_xml
int32_t KVM::Init() {
    // set name, img, iso
    SetName();
    stringstream ss;
    ss << GetId();
    m_dir = FLAGS_libvirt_dir + "/" + GetName() + "/";
    m_img = m_dir + "kvm_" + ss.str() + ".img";
    m_iso = m_dir + "kvm_" + ss.str() + ".iso";
    m_conf = m_dir + "CONF";

    // mkdir work dir
    if (access(m_dir.c_str(), F_OK) == -1) {
        if (mkdir(m_dir.c_str(), 0755) != 0) {
           LOG4CPLUS_ERROR(logger, "can't create vm work dir");
           return -1;
        }
    }

    // build connection
    if (NULL == m_conn) {
        m_conn = virConnectOpen("qemu:///system");
        if(NULL == m_conn) {
            LOG4CPLUS_ERROR(logger, "fails to open connection to qemu:///system");
            return -1;
        }
    }

    // read xml template into m_xml_template
    if ("" == m_xml_template) {
        // open libvirt xml template
        ifstream file(FLAGS_xml_template.c_str());
        if (!file) {
            LOG4CPLUS_ERROR(logger, "can't read xml template file");
            return -1;
        }

        // read xml template content into m_xml
        stringstream ss;
        ss << file.rdbuf();
        m_xml_template = ss.str();
    }

    // set m_xml
    m_xml = m_xml_template;
    cout << "in Init()" << endl;

    return 0;
}

// cp img, type = raw
int32_t KVM::CopyImage() {
    // img is exist?
    string img_template = FLAGS_libvirt_dir + GetTaskInfo().vm_info.img_template;
    if (access(img_template.c_str(), F_OK) == -1) {
        LOG4CPLUS_ERROR(logger, "template " << img_template << " dose not exits");
        return -1;
    }

    // cp img
    string cmd = "cp " + img_template + " " + m_img;
    int32_t ret = system(cmd.c_str());
    ret = ret >> 8;
    printf("ret: %d\n", ret);
    if (ret != 0) {
        LOG4CPLUS_ERROR(logger, "can't copy image template");
        return -1;
    }
    return 0;
}

// clone img from img template, type = qcow2 .qco
int32_t KVM::CloneImage() {
    // img is exist?
    string img_template = FLAGS_libvirt_dir + GetTaskInfo().vm_info.img_template;
    if (access(img_template.c_str(), F_OK) == -1) {
        LOG4CPLUS_ERROR(logger, "template " << img_template << " dose not exits");
        return -1;
    }

    // cp img
    string cmd = "qemu-img create -b " + img_template + " -f qcow2 " + m_img + " > /dev/null 2>&1";
    int32_t ret = system(cmd.c_str());
    ret = ret >> 8;
    printf("ret: %d\n", ret);
    if (ret != 0) {
        LOG4CPLUS_ERROR(logger, "can't clone image template");
        return -1;
    }
    return 0;
 
}

// customize libvirt xml config file according @vm_info
int32_t KVM::ConfigVirXML() {
    // read xml content into xml_conf
    string xml_conf = m_xml;

    // customize xml
    // name
    string::size_type pos = xml_conf.find("T_NAME");
    if (pos == string::npos) {
        LOG4CPLUS_ERROR(logger, "error in finding T_NAME in kvm xml template");
        return -1;
    }
    xml_conf.replace(pos, strlen("T_NAME"), GetName());

    // memory
    pos = xml_conf.find("T_MEMORY");
    if (pos == string::npos) {
        LOG4CPLUS_ERROR(logger, "error in finding T_MEMORY in kvm xml template");
        return -1;
    }

    stringstream ss;
    ss.str("");
    ss.clear();
    ss << (GetTaskInfo().vm_info.memory * 1024);
    xml_conf.replace(pos, strlen("T_MEMORY"), ss.str());
  
    // vcpu 
    pos = xml_conf.find("T_VCPU");
    if (pos == string::npos) {
        LOG4CPLUS_ERROR(logger, "error in finding T_VCPU in kvm xml template");
        return -1;
    }
    ss.str("");
    ss.clear();
    //vcpu以及cpu_shares目前是有联系的，对于kvm来说，CPU最好是个整数
    ss << GetTaskInfo().vm_info.vcpu;
    xml_conf.replace(pos, strlen("T_VCPU"), ss.str());

    /* // CPU_SHARE
    pos = xml_copy.find("T_CPU_SHARE");
    if (pos == string::npos) {
        LOG4CPLUS_ERROR(logger, "error in finding T_CPU_SHARE in vm template");
        return -1;
    }
    ss.str("");
    ss.clear();
    // cpu 参数设置的是cpu的share值
    // 一个cpu对应的归一化参数是1024 
    ss << GetTaskInfo().vm_info.vcpu * 1024;
    xml_conf.replace(pos, strlen("T_CPU_SHARE"), ss.str());
    */

    // pos = xml_copy.find("T_OUT_BOUND");
    // pos = xml_copy.find("T_IN_BOUND");

    // boot
    pos =  xml_conf.find("T_BOOT");
    if (pos == string::npos) {
        LOG4CPLUS_ERROR(logger, "error in finding T_BOOT in kvm xml template");
        return -1;
    }
    if (!GetTaskInfo().vm_info.iso_location.empty()) {
        // start from iso 
        xml_conf.replace(pos, strlen("T_BOOT"), "cdrom");
    } else {
        // start from hard disk
        xml_conf.replace(pos, strlen("T_BOOT"), "hd");
    }

    // img
    pos = xml_conf.find("T_IMG_LOCATION");
    if (pos == string::npos) {
        LOG4CPLUS_ERROR(logger, "error in finding T_IMG_LOCATION in kvm xml template");
        return -1;
    }
    xml_conf.replace(pos, strlen("T_IMG_LOCATION"), m_img);

    // iso
    pos = xml_conf.find("T_ISO_LOCATION");
    if (pos == string::npos) {
        LOG4CPLUS_ERROR(logger, "error in finding T_ISO_LOCATION in kvm xml template");
        return -1;
    }
    xml_conf.replace(pos, strlen("T_ISO_LOCATION"), m_iso);

    // vnc port
    pos =  xml_conf.find("T_VNC_PORT");
    if (pos == string::npos) {
        LOG4CPLUS_ERROR(logger, "error in finding T_VNC_PORT in kvm xml template");
        return -1;
    }
    ss.str("");
    ss.clear();
    ss << GetTaskInfo().vm_info.vnc_port + 5900;
    xml_conf.replace(pos, strlen("T_VNC_PORT"), ss.str());
   
    m_xml = xml_conf; 

    return 0;
}

// create libvirt kvm
int32_t KVM::CreateVM() {
    // task is exist?
    TaskPtr task_ptr = GetTaskPtr();
    int64_t task_id = GetId();
 
    cout << 1 << endl;    
    // TaskPtr task_ptr = TaskPoolI::Instance()->GetTaskPtr(task_id);
    // can't find the task, taskptr = NULL
    if (!task_ptr) {
        LOG4CPLUS_ERROR(logger, "can't find task " << task_id);
        task_ptr->SetStates(TaskEntityState::TASKENTITY_FAILED, 0.0);
        return -1;
    }

    cout << 2 << endl;
    // init, set name img iso, mk work dir, cp libvirt xml
    if (Init() != 0) {
        LOG4CPLUS_ERROR(logger, "can't create init vm");
        cout << 2.5 << endl;
        task_ptr->SetStates(TaskEntityState::TASKENTITY_FAILED, 0.0);
        return -1;
    }
    task_ptr->SetStates(TaskEntityState::TASKENTITY_STARTING, 5.0);

    cout << 3 << endl;
    // clone img
    if (CloneImage() != 0) {
        LOG4CPLUS_ERROR(logger, "can't clone image");
        task_ptr->SetStates(TaskEntityState::TASKENTITY_FAILED, 0.0);
        return -1;
    }
    task_ptr->SetStates(TaskEntityState::TASKENTITY_STARTING, 25.0);

    cout << 4 << endl;
    // config iso, include ip, app
    ofstream conf_file(m_conf.c_str());
    conf_file << "[vm_agent]" << endl;
    conf_file << "vm_id = " << GetId() << endl;
    conf_file << "os = " << GetTaskInfo().vm_info.os << endl;
    conf_file << "ip = " << GetTaskInfo().vm_info.ip << endl;
    conf_file << "port = " << GetTaskInfo().vm_info.port << endl;
    conf_file << "executor_endpoint = " 
              << ResourceMgrI::Instance()->GetBridgeEndpoint() << endl;
    conf_file << "heartbeat_interval = " << FLAGS_vm_hb_interval << endl;
    conf_file.close();
    
    string cmd = "mkisofs -o " + m_iso + " " + m_conf + " > /dev/null 2>&1";
    int32_t ret = system(cmd.c_str());
    ret = ret >> 8;
    if (ret != 0) {
        LOG4CPLUS_ERROR(logger, "can't create conf iso file");
        task_ptr->SetStates(TaskEntityState::TASKENTITY_FAILED, 0.0);
        return -1;
    }
    task_ptr->SetStates(TaskEntityState::TASKENTITY_STARTING, 30.0);

    cout << 5 << endl;
    // libvirt template XML
    if (ConfigVirXML() != 0) {
        LOG4CPLUS_ERROR(logger, "can't customize kvm xml");
        task_ptr->SetStates(TaskEntityState::TASKENTITY_FAILED, 0.0);
        return -1;
    }
    task_ptr->SetStates(TaskEntityState::TASKENTITY_STARTING, 35.0);
    cout << m_xml << endl;  
 
    cout << 6 << endl; 
    // create vm
    m_domain_ptr = virDomainCreateXML(m_conn, m_xml.c_str(), 0);
    if (!m_domain_ptr) {
        virErrorPtr error = virGetLastError();
        LOG4CPLUS_ERROR(logger, error->message);
        task_ptr->SetStates(TaskEntityState::TASKENTITY_FAILED, 0.0);
        return -1;
    }
    task_ptr->SetStates(TaskEntityState::TASKENTITY_STARTING, 40.0);
   
    cout << 7 << endl; 
    return 0;    
}

// after create vm
int32_t KVM::SetVNetByXML() {
    // get XML by vir_ptr
    char *xml = virDomainGetXMLDesc(m_domain_ptr, 0);

    // parse xml doc
    xml_document<> doc;
    try {
        file<> fdoc(xml);
        doc.parse<0>(fdoc.data());
        // FATAL: free xml at the end
    } catch (rapidxml::parse_error& ex) {
        LOG4CPLUS_ERROR(logger, "synex error in " << ex.what());
        return -1;
    } catch (std::runtime_error& ex) {
        LOG4CPLUS_ERROR(logger, "vm xml error:" << ex.what());
        return -1;
    }

    // parse, get vnet from xml
    xml_node<> *root = doc.first_node("domain");
    if (!root) {
        LOG4CPLUS_ERROR(logger, "no domain found.");
        return -1;
    }

    xml_node<> *node_level1,*node_level2, *node_level3;
    xml_attribute<> *attr;

    node_level1 = root->first_node("devices");
    if (!node_level1) {
        LOG4CPLUS_ERROR(logger, "no devices found.");
        return -1;
    }

    node_level2 = node_level1->first_node("interface");
    if (!node_level2) {
        LOG4CPLUS_ERROR(logger, "no interface found.");
        return -1;
    }

    node_level3 = node_level2->first_node("target");
    if (!node_level3) {
        LOG4CPLUS_ERROR(logger, "no target found.");
        return -1;
    }

    attr = node_level3->first_attribute("dev");
    if (!attr) {
        LOG4CPLUS_ERROR(logger, "no interface dev found.");
        return -1;
    }

    // set vnet
    SetVNet(attr->value());
    free(xml);
    return 0;
}


// install app into kvm
int32_t KVM::Install() {
    return 0;
}


// get hb resource

/*ExecutorStat VM::GetUsedResource() {
    ExecutorStat stat;
    stat.cpu_usage = GetCpuUsage();
    stat.memory_usage = GetMemoryUsage();
    stat.io_usage = GetIOUsage();
    stat.vc_name = m_info.vc_name;
    stat.task_id = m_info.id;
    return stat;
}*/

double KVM::GetCpuUsage() {
    double usage = 0.0;
    double cur_cpu = 0.0, cur_total = 0.0;
    virDomainInfo vir_info;
    struct timeval real_time;

    // get current cpu time
    if (virDomainGetInfo(m_domain_ptr, &vir_info) != 0) {
        LOG4CPLUS_ERROR(logger, "can't get domain info!");
        return 0.0;
    }
    // us, wei miao 
    cur_cpu = vir_info.cpuTime / 1000;

    // get current time
    if (gettimeofday(&real_time, NULL) == -1) {
        LOG4CPLUS_ERROR(logger, "can't get real time!");
        return 0.0;
    }
    // us, wei miao
    cur_total = 1000000 * real_time.tv_sec + real_time.tv_usec;

    // is first
    if (m_first) {
        m_prev_cpu = cur_cpu;
        m_prev_total = cur_total;
        m_first = false;
        return 0.0;
    }
    // not first
    usage = static_cast<double> 
            (cur_cpu - m_prev_cpu) / (cur_total - m_prev_total);
    m_prev_cpu = cur_cpu;
    m_prev_total = cur_total;
    
    if (usage < 0)
        usage = 0.0;

    return usage;
    
}

//TODO
double KVM::GetMemoryUsage() {
    return 0;
}

//TODO
//double KVM::GetDiskUsage() {
//    return 0;
//}

// TODO
//int32_t VM::Recycle() {
    // return virDomainDestroy(m_domain_ptr);
//}
