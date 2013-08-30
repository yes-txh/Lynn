/********************************
 FileName: executor/kvm.cpp
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: kvm, inherit from vm
*********************************/

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <gflags/gflags.h>

#include "executor/kvm.h"
#include "executor/task_entity_pool.h"

using std::stringstream;
using log4cplus::Logger;
using lynn::WriteLocker;
using lynn::ReadLocker;

// gflag img_dir = /var/lib/libvirt/images/
DECLARE_string(img_dir);

static Logger logger = Logger::getInstance("executor");

/*KVM::KVM(const TaskInfo& info) : VM(info) {
}

KVM::~KVM() {
    if(m_domain_ptr) {
        virDomainFree(m_domain_ptr);
    }
}*/

// private

int32_t KVM::Init() {
    // set img path
    stringstream ss;
    ss << GetId();
    m_img = FLAGS_img_dir + "/" + ss.str() + "/" + ss.str() + ".img";
    m_iso = FLAGS_img_dir + "/" + ss.str() + "/" + ss.str() + ".iso";

    return 0;
}
// cp img, type = raw
int32_t KVM::CopyImage() {
    // img is exist?
    string img_template = FLAGS_img_dir + GetTaskInfo().vm_info.img_template;
    if (access(img_template.c_str(), F_OK) == -1) {
        LOG4CPLUS_ERROR(logger, "template " << img_template << " dose not exits");
        return -1;
    }

    // cp img
    string cmd = "cp" + img_template + " " + m_img;
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
    string img_template = FLAGS_img_dir + GetTaskInfo().vm_info.img_template;
    if (access(img_template.c_str(), F_OK) == -1) {
        LOG4CPLUS_ERROR(logger, "template " << img_template << " dose not exits");
        return -1;
    }

    // cp img
    string cmd = "qemu-img create -b" + img_template + " -f qcow2" + m_img + " > /dev/null 2>&1";
    int32_t ret = system(cmd.c_str());
    ret = ret >> 8;
    printf("ret: %d\n", ret);
    if (ret != 0) {
        LOG4CPLUS_ERROR(logger, "can't clone image template");
        return -1;
    }
    return 0;
 
}
int32_t KVM::CreateVM() {
    int64_t task_id = GetId();
    TaskPtr task_ptr = TaskPoolI::Instance()->GetTaskPtr(task_id);
    // can't find the task, taskptr = NULL
    if (!task_ptr) {
        LOG4CPLUS_ERROR(logger, "can't find task " << task_id);
        printf("can't find task %ld\n", task_id);
        task_ptr->SetStates(TaskEntityState::TASKENTITY_FAILED, 0.0);
        return -1;
    }

      
    return 0;
}
