/********************************
 FileName: executor/task_entity.h
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: task entity in executor, corresponds to task, corresponds to kvm/lxc
*********************************/

#ifndef SRC_EXECUTOR_TASKENTITY_H
#define SRC_EXECUTOR_TASKENTITY_H

#include <string>
#include <boost/shared_ptr.hpp>

#include "include/proxy.h"
#include "common/clynn/rwlock.h"
#include "executor/type.h"

using std::string;
using boost::shared_ptr;
using clynn::RWLock;

class TaskEntity {
public:
    explicit TaskEntity(const string& task_info, bool& ret);

    ~TaskEntity() {} 

    TaskID GetID() {
        return m_id;
    }

    VMType::type GetVMType() {
        return m_info.type;
    }

    TaskEntityState::type GetState();

    double GetPercentage();

    bool SetStates(const TaskEntityState::type state, const double percentage);

    bool SetState(const TaskEntityState::type state);

    bool SetPercentage(const double percentage);

    // call back
    void TaskStarted();

    void TaskFinished();

    void TaskFailed();

    // core operation
    bool Start();

    bool Stop();

    bool Kill();

private:
    TaskID m_id;
    TaskInfo m_info;               // from executor/type.h
    TaskEntityState::type m_state; // from proxy/executor/executor.thrift
    RWLock m_lock;
    double m_percentage;
};

typedef shared_ptr<TaskEntity> TaskPtr;

#endif  
