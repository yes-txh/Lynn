/********************************
 FileName: include/classad_attr.h
 Author:   WangMin
 Date:     2013-09-16
 Version:  0.1
 Description: attributes of classad
*********************************/

#ifndef SRC_INCLUDE_CLASSAD_ATTR_H
#define SRC_INCLUDE_CLASSAD_ATTR_H

using std::string;

/// @brief: TaskInfo
// task info overview
static const string ATTR_JOB_ID = "JOB_ID";
static const string ATTR_TASK_ID = "TASK_ID";
static const string ATTR_VMTYPE = "VMTYPE";
static const string ATTR_IS_RUN = "IS_RUN";

// task vminfo
static const string ATTR_MEMORY = "MEMORY";
static const string ATTR_VCPU = "VCPU";
static const string ATTR_IP = "IP";
static const string ATTR_PORT = "PORT";
static const string ATTR_OS = "OS";
// -- only for kvm
static const string ATTR_IMG = "IMG";
static const string ATTR_ISO = "ISO";
static const string ATTR_SIZE = "SIZE";
static const string ATTR_VNC_PORT = "VNC_PORT";

// task appinfo
static const string ATTR_APP_ID = "APP_ID";
static const string ATTR_APP_NAME = "APP_NAME";
// -- outside vm, hdfs
static const string ATTR_APP_SRC_PATH = "SRC_PATH";
static const string ATTR_APP_OUT_DIR = "OUT_DIR";
// -- inside vm, windows or linux
static const string ATTR_INSTALL_DIR = "INSTALL_DIR";
static const string ATTR_EXE_PATH = "EXE_PATH";
static const string ATTR_STOP_PATH = "STOP_PATH";
static const string ATTR_OUT_DIR = "EXE_OUT_DIR";

/// @brief: Register MachineInfo
static const string ATTR_Machine = "Machine";
static const string ATTR_MachineType = "MachineType";
static const string ATTR_Shelf = "Shelf";
static const string ATTR_Machine_IP = "IP";
static const string ATTR_Port = "Port";
static const string ATTR_Arch = "Arch";
static const string ATTR_OpSys = "OpSys";
static const string ATTR_TotalCPUNum = "TotalCPUNum";
static const string ATTR_TotalMemory = "TotalMemory";
static const string ATTR_TotalDisk = "TotalDisk";
static const string ATTR_BandWidth = "BandWidth";
static const string ATTR_NICType = "NICType";

#endif
