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
using clynn::WriteLocker;
using clynn::ReadLocker;

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
    if (CreateKVM() != 0) {
        LOG4CPLUS_ERROR(logger, "Fails to create kvm, name:" << GetName() << ", job_id:" << GetID().job_id << ", task_id:" << GetID().task_id); 
        return -1;
    }

    /*if (Install() != 0) {
        LOG4CPLUS_ERROR(logger, "Fails to install app into kvm, name:" << GetName() << ", job_id:" << GetID().job_id << ", task_id:" << GetID().task_id 
                        << ", app:" << GetAppName());
        return -1;
    }*/

    LOG4CPLUS_INFO(logger, "Create kvm successfully, name:" << GetName() << ", job_id:" << GetID().job_id << ", task_id:" << GetID().task_id);
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
    if (!m_domain_ptr) {
        LOG4CPLUS_INFO(logger, "Invalid domain pointer, Kill VM directly.");
        // delete work dir
        string cmd = "rm -r " + m_dir;
        system(cmd.c_str());
        return true;
    }

    if (virDomainDestroy(m_domain_ptr) != 0) {
        virErrorPtr error = virGetLastError();
        LOG4CPLUS_ERROR(logger, error->message);
        LOG4CPLUS_ERROR(logger, "Can't kill kvm, name:" << GetName() << ", job_id:" << GetID().job_id << ", task_id:" << GetID().task_id);
        return false;
    }

    // delete work dir
    string cmd = "rm -r " + m_dir;
    system(cmd.c_str());
    return true;
}

HbVMInfo KVM::GetHbVMInfo() {
    VMState::type state = GetState();
    // if state != VM_SERVICE_ONLINE then return "empty"
    if (true || state != VMState::VM_SERVICE_ONLINE){
       HbVMInfo empty;
       empty.id = GetID();
       empty.name = GetName();
       empty.type = GetVMType();
       empty.cpu_usage = 0;
       empty.memory_usage = 0;
       empty.bytes_in = 0;
       empty.bytes_out = 0;
       empty.state = state;
       empty.app_running = false;
       return empty;
    }
    // state == VM_SERVICE_ONLINE
    ReadLocker locker(GetLock());
    return m_hb_vm_info;
}

VMState::type KVM::GetState() {
    ReadLocker locker(GetLock());
    // heartbeat is available
    if ((m_timestamp != -1) && (time(NULL) - m_timestamp < m_time_to_death)) {
        return m_hb_vm_info.state;
    }
    
    // time out, heartbeat is not available
    int state;
    virDomainGetState(m_domain_ptr, &state, 0, 0);
    if(state == VIR_DOMAIN_RUNNING) {
        // vm is running, get state with libvirt
        return VMState::VM_ONLINE;
    }

    return VMState::VM_OFFLINE;
}

void KVM::SetHbVMInfo(const VM_HbVMInfo& hb_vm_info) {
    WriteLocker locker(GetLock());
    m_hb_vm_info.cpu_usage = hb_vm_info.cpu_usage;
    m_hb_vm_info.memory_usage = hb_vm_info.memory_usage;
    m_hb_vm_info.bytes_in = hb_vm_info.bytes_in; 
    m_hb_vm_info.bytes_out = hb_vm_info.bytes_out;
    m_hb_vm_info.state = hb_vm_info.state;
    m_hb_vm_info.app_running = hb_vm_info.app_running;
    m_timestamp = time(NULL);
}

/// unique in KVM 
virDomainPtr KVM::GetDomainPtr() const {
    return m_domain_ptr;
}

//void KVM::SetDomainPtr(virDomainPtr ptr) {
//    m_domain_ptr = ptr;
//}

int32_t KVM::GetVNCPort() const {
    return m_vnc_port;
}

//void KVM::SetVNCPort(int32_t port) {
//    m_vnc_port = port;
//}

string KVM::GetVNet() const {
    return m_vnet;
}

void KVM::SetVNet(string vnet) {
    m_vnet = vnet;
}


/// @brief: private function
// set name
void KVM::SetName() {
    // app_name + "_kvm_" + job_id + "_" + task_id
    TaskID id = GetID();
    stringstream ss_job, ss_task;
    ss_job << id.job_id;
    ss_task << id.task_id;
    string name = GetTaskInfo().app_info.name + "_kvm_" + ss_job.str() + "_" + ss_task.str();
    SetNameByString(name);
}

// init heartbeat
void KVM::InitHeartbeat() {
    m_hb_vm_info.id = GetID();
    m_hb_vm_info.name = GetName();
    m_hb_vm_info.type = GetVMType();
}

// set name heartbeat img iso, mk work dir, build libvirt connection, read xml_template into m_xml
int32_t KVM::Init() {
    // set name
    SetName();
    // init hb
    InitHeartbeat();

    // set dir, img, iso
    TaskID id = GetID();
    stringstream ss_job, ss_task;
    ss_job << id.job_id;
    ss_task << id.task_id;
    m_dir = FLAGS_libvirt_dir + "/" + GetName() + "/";
    m_img = m_dir + "kvm_" + ss_job.str() + "_" + ss_task.str() + ".img";
    m_iso = m_dir + "kvm_" + ss_job.str() + "_" + ss_task.str() + ".iso";
    m_conf = m_dir + "CONF";

     
    // check total libvirt work directory
    if (chdir(FLAGS_libvirt_dir.c_str()) < 0) {
        LOG4CPLUS_ERROR(logger, "No libvirt work directory:" << FLAGS_libvirt_dir);
        return -1;
    }
    // mkdir work dir
    if (access(m_dir.c_str(), F_OK) == -1) {
        if (mkdir(m_dir.c_str(), 0755) != 0) {
           LOG4CPLUS_ERROR(logger, "Can't create kvm work dir:" << m_dir);
           return -1;
        }
    }

    // build connection
    if (NULL == m_conn) {
        m_conn = virConnectOpen("qemu:///system");
        if(NULL == m_conn) {
            LOG4CPLUS_ERROR(logger, "Fails to open connection to qemu:///system");
            return -1;
        }
    }

    // read xml template into m_xml_template
    if ("" == m_xml_template) {
        // open libvirt xml template
        ifstream file(FLAGS_xml_template.c_str());
        if (!file) {
            LOG4CPLUS_ERROR(logger, "Can't read xml template file: " << FLAGS_xml_template);
            return -1;
        }

        // read xml template content into m_xml
        stringstream ss;
        ss << file.rdbuf();
        m_xml_template = ss.str();
    }

    // set m_xml
    m_xml = m_xml_template;

    return 0;
}

// cp img, type = raw
int32_t KVM::CopyImage() {
    // img is exist?
    string img_template = FLAGS_libvirt_dir + GetTaskInfo().vm_info.img_template;
    if (access(img_template.c_str(), F_OK) == -1) {
        LOG4CPLUS_ERROR(logger, "Template " << img_template << " dose not exits");
        return -1;
    }

    // cp img
    string cmd = "cp " + img_template + " " + m_img;
    int32_t ret = system(cmd.c_str());
    ret = ret >> 8;
    if (ret != 0) {
        LOG4CPLUS_ERROR(logger, "Can't copy image template");
        return -1;
    }
    return 0;
}

// clone img from img template, type = qcow2 .qco
int32_t KVM::CloneImage() {
    // img is exist?
    string img_template = FLAGS_libvirt_dir + GetTaskInfo().vm_info.img_template;
    if (access(img_template.c_str(), F_OK) == -1) {
        LOG4CPLUS_ERROR(logger, "Template " << img_template << " dose not exits");
        return -1;
    }

    // cp img
    string cmd = "qemu-img create -b " + img_template + " -f qcow2 " + m_img + " > /dev/null 2>&1";
    int32_t ret = system(cmd.c_str());
    ret = ret >> 8;
    if (ret != 0) {
        LOG4CPLUS_ERROR(logger, "Can't clone image template");
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
        LOG4CPLUS_ERROR(logger, "Error in finding T_NAME in kvm xml template");
        return -1;
    }
    xml_conf.replace(pos, strlen("T_NAME"), GetName());

    // memory
    pos = xml_conf.find("T_MEMORY");
    if (pos == string::npos) {
        LOG4CPLUS_ERROR(logger, "Error in finding T_MEMORY in kvm xml template");
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
        LOG4CPLUS_ERROR(logger, "Error in finding T_VCPU in kvm xml template");
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
        LOG4CPLUS_ERROR(logger, "Error in finding T_BOOT in kvm xml template");
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
        LOG4CPLUS_ERROR(logger, "Error in finding T_IMG_LOCATION in kvm xml template");
        return -1;
    }
    xml_conf.replace(pos, strlen("T_IMG_LOCATION"), m_img);

    // iso
    pos = xml_conf.find("T_ISO_LOCATION");
    if (pos == string::npos) {
        LOG4CPLUS_ERROR(logger, "Error in finding T_ISO_LOCATION in kvm xml template");
        return -1;
    }
    xml_conf.replace(pos, strlen("T_ISO_LOCATION"), m_iso);

    // vnc port
    pos =  xml_conf.find("T_VNC_PORT");
    if (pos == string::npos) {
        LOG4CPLUS_ERROR(logger, "Error in finding T_VNC_PORT in kvm xml template");
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
int32_t KVM::CreateKVM() {
    // task is exist?
    TaskID id = GetID();
    TaskPtr task_ptr = GetTaskPtr();
    // TODO 
    // TaskPtr task_ptr = TaskPoolI::Instance()->GetTaskPtr(task_id);
    // can't find the task, taskptr = NULL
    if (!task_ptr) {
        LOG4CPLUS_ERROR(logger, "Can't find task, job_id:" << id.job_id << ", task_id" << id.task_id);
        task_ptr->TaskFailed();
        return -1;
    }

    // init, set name img iso, mk work dir, cp libvirt xml
    if (Init() != 0) {
        LOG4CPLUS_ERROR(logger, "Can't create init vm");
        task_ptr->TaskFailed();        
        return -1;
    }
    task_ptr->SetStates(TaskEntityState::TASKENTITY_STARTING, 5.0);

    // clone img
    if (CloneImage() != 0) {
        LOG4CPLUS_ERROR(logger, "Can't clone image");
        task_ptr->TaskFailed();
        return -1;
    }
    task_ptr->SetStates(TaskEntityState::TASKENTITY_STARTING, 25.0);

    // config iso, include ip, app
    ofstream conf_file(m_conf.c_str());
    conf_file << "[vm_agent]" << endl;
    conf_file << "job_id = " << GetID().job_id << endl;
    conf_file << "task_id = " << GetID().task_id << endl;
    conf_file << "name = " << GetName() << endl;
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
        LOG4CPLUS_ERROR(logger, "Can't create conf iso file");
        task_ptr->TaskFailed();
        return -1;
    }
    task_ptr->SetStates(TaskEntityState::TASKENTITY_STARTING, 30.0);

    // libvirt template XML
    if (ConfigVirXML() != 0) {
        LOG4CPLUS_ERROR(logger, "Can't customize kvm xml");
        task_ptr->TaskFailed();
        return -1;
    }
    task_ptr->SetStates(TaskEntityState::TASKENTITY_STARTING, 35.0);
 
    // create vm
    m_domain_ptr = virDomainCreateXML(m_conn, m_xml.c_str(), 0);
    if (!m_domain_ptr) {
        virErrorPtr error = virGetLastError();
        LOG4CPLUS_ERROR(logger, "Can't create kvm by libvirt xml," << error->message);
        task_ptr->TaskFailed();
        return -1;
    }
    task_ptr->SetStates(TaskEntityState::TASKENTITY_STARTING, 40.0);
   
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
int32_t KVM::InstallApp() {
    return 0;
}

int32_t KVM::StartApp() {
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

/* double KVM::GetCpuUsage() {
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
    
} */

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
