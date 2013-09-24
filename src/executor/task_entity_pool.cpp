/********************************
 FileName: executor/task_entity_pool.cpp
 Author:   WangMin
 Date:     2013-08-27
 Version:  0.1
 Description: the pool of taskentitys
*********************************/

#include <assert.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "executor/task_entity_pool.h"

using log4cplus::Logger;
using clynn::ReadLocker;
using clynn::WriteLocker;

static Logger logger = Logger::getInstance("executor");

// TODO just for test
void TaskEntityPool::PrintAll() {
    ReadLocker locker(m_lock);
    printf("task_entity_pool.cpp\n");
    printf("************ Task Entity ************\n");
    for (map<TaskID, TaskPtr>::iterator it = m_task_map.begin();
         it != m_task_map.end(); ++it) {
        printf("taskentity job_id:%d, task_id:%d\n", (it->first).job_id, (it->first).task_id);
    }    
    printf("**************************************\n");
}

bool TaskEntityPool::Find(const TaskPtr& ptr) {
    ReadLocker locker(m_lock);
    // ptr->GetID() is TaskID(job_id, task_id)
    map<TaskID, TaskPtr>::iterator it = m_task_map.find(ptr->GetID());
    return it != m_task_map.end();
}

void TaskEntityPool::Insert(const TaskPtr& ptr) {
    WriteLocker locker(m_lock);
    m_task_map[ptr->GetID()] = ptr;
}

void TaskEntityPool::Delete(const TaskID id ) {
    WriteLocker locker(m_lock);
    // erase() will invoke destructor(xi gou) func
    m_task_map.erase(id);
}

// @id is the key, @func is the function pointer
bool TaskEntityPool::FindToDo(const TaskID id, TaskFunc func) {
    ReadLocker locker(m_lock);
    // find the task
    map<TaskID, TaskPtr>::iterator it = m_task_map.find(id);
    if (it != m_task_map.end()) {
        // do sth by func, func is a function pointer
        // func needs to be assigned
        func((it->second).get());
        return true;
    }
    
    return false;    
}

// travesal m_task_map by order, start the first waiting taskEntity
// other taskentitys will be started periodically
void TaskEntityPool::StartTask() {
    ReadLocker locker(m_lock);
    for (map<TaskID, TaskPtr>::iterator it = m_task_map.begin();
         it != m_task_map.end(); ++it) {
        if ((it->second)->GetState() == TaskEntityState::TASKENTITY_WAITING) {
            if (false == (it->second)->Start()) {
                WriteLocker locker(m_lock);
                // task failed
                (it->second)->TaskFailed();
                LOG4CPLUS_ERROR(logger, "Fails to start the task, job_id:" << (it->second)->GetID().job_id << ", task_id:" << (it->second)->GetID().task_id);
            } else
                return;
        }
    }
}

// bind a function, then invoke it with FindToDo
// kill a taskEntity, and delete taskPtr from m_task_map
bool TaskEntityPool::KillTaskByID(const TaskID id) {
    //TODO
    PrintAll();

    // get task
    TaskPtr ptr = GetTaskPtr(id);
    // no the task
    if (!ptr) {
        LOG4CPLUS_ERROR(logger, "Can't find the task, fails to kill task, job_id:" << id.job_id << ", task_id:" << id.task_id);
        return false;
    }
     
    // fails to kill task
    if (!(ptr->Kill())) {
        LOG4CPLUS_ERROR(logger, "Fails to kill task, job_id:" << id.job_id << ", task_id:" << id.task_id);
        return false;
    }

    // delete task from pool(map)
    // Delete(id);
    // LOG4CPLUS_INFO(logger, "Kill task successfully, job_id:" << id.job_id << ", task_id:" << id.task_id);
    PrintAll();
    return true;
}

TaskPtr TaskEntityPool::GetTaskPtr(const TaskID id) {
    ReadLocker locker(m_lock);
    map<TaskID, TaskPtr>::iterator it = m_task_map.find(id);
    if (it != m_task_map.end()) {
        return it->second;
    }
    // not find then return NULL
    return TaskPtr();
}
