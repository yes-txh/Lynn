/********************************
 FileName: executor/resource_manager.cpp
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: resource manager of the machine(execute node)
*********************************/

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <gflags/gflags.h>
#include <classad/classad.h>
#include <classad/classad_distribution.h>

#include "executor/system.h"
#include "executor/resource_manager.h"

using std::stringstream;
using log4cplus::Logger;
using lynn::ReadLocker;
using lynn::WriteLocker;

DECLARE_int32(port);
DECLARE_string(collector_endpoint);
DECLARE_string(interface);
DECLARE_string(if_bridge);

static Logger logger = Logger::getInstance("executor");

// init, set static info 
// TODO
bool ResourceManager::Init() {
    /// @brief: set private info
    // ip
    m_ip = System::GetIP(FLAGS_interface.c_str());
    m_bridge_ip = System::GetIP(FLAGS_if_bridge.c_str());
    m_port = FLAGS_port;

    // machine & arch
    m_name = m_ip;
    m_machine_type = "A";
    m_shelf_number = -1;
    m_arch = "Intel";
    m_os = System::GetOSVersion();

    m_total_cpu = System::GetCpuNum();
    m_total_memory = System::GetTotalMemory(); 
    m_total_disk = System::GetTotalDisk();
    m_avail_cpu = m_total_cpu;
    m_avail_memory = m_total_memory;
    m_avail_disk = m_total_disk - System::GetUsedDisk();

    m_band_width = System::GetBandWidth(FLAGS_interface.c_str());
    m_nic_type = System::GetNICType(FLAGS_interface.c_str());

    /// @brief: get info, send info to collector
    string machine_info = GetMachineInfo();
    // send info to collector
    try {
        Proxy<CollectorClient> proxy = Rpc<CollectorClient, CollectorClient>::GetProxy(FLAGS_collector_endpoint);
        if(!proxy().RegistMachine(machine_info)){
             LOG4CPLUS_ERROR(logger, "register machine failed");
            return false;
        }
    } catch (TException &tx) {
        LOG4CPLUS_ERROR(logger, "rpc error: register machine failed" << tx.what());
        return false;
    }
      
    return true;
}

string ResourceManager::GetMachineInfo() {
    ReadLocker lock(m_lock);
    
    // init classad
    ClassAd ad;
    ad.InsertAttr("Machine", m_name);
    ad.InsertAttr("MachineType", m_machine_type);
    ad.InsertAttr("Shelf", m_shelf_number);
    ad.InsertAttr("IP", m_ip);
    ad.InsertAttr("Port", m_port);
    ad.InsertAttr("Arch", m_arch);
    ad.InsertAttr("OpSys", m_os);
    ad.InsertAttr("TotalCPUNum", m_total_cpu);
    ad.InsertAttr("TotalMemory", m_total_memory);
    ad.InsertAttr("TotalDisk", m_total_disk);
    ad.InsertAttr("BandWidth", m_band_width);
    ad.InsertAttr("NICType", m_nic_type);
  
    // classad -> string
    ClassAdUnParser unparser;
    string str_ad;
    // Serialization, convert ClassAd into string str_ad
    unparser.Unparse(str_ad, &ad);
    
    return str_ad; 
}

string ResourceManager::GetBridgeEndpoint() {
    stringstream ss;
    ss << m_port;
    return m_bridge_ip + ":" + ss.str();
}

void ResourceManager::AllocateResource(double cpu, int32_t memory, int32_t disk) {
    WriteLocker locker(m_lock);
    m_avail_cpu -= cpu;
    m_avail_memory -= memory;
    m_avail_disk -= disk;
}

void ResourceManager::ReleaseResource(double cpu, int32_t memory, int32_t disk) {
    WriteLocker locker(m_lock);
    m_avail_cpu += cpu;
    m_avail_memory += memory;
    m_avail_disk += disk;
}
