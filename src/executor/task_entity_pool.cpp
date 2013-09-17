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
    for (map<int64_t, TaskPtr>::iterator it = m_task_map.begin();
         it != m_task_map.end(); ++it) {
        printf("taskentity: %ld\n", it->first);
    }    
    printf("**************************************\n");
}

bool TaskEntityPool::Find(const TaskPtr& ptr) {
    ReadLocker locker(m_lock);
    // ptr->GetId() is taskid(int64_t)
    map<int64_t, TaskPtr>::iterator it = m_task_map.find(ptr->GetId());
    return it != m_task_map.end();
}

void TaskEntityPool::Insert(const TaskPtr& ptr) {
    WriteLocker locker(m_lock);
    m_task_map[ptr->GetId()] = ptr;
}

void TaskEntityPool::Delete(const int64_t id ) {
    WriteLocker locker(m_lock);
    // erase() will invoke destructor(xi gou) func
    m_task_map.erase(id);
}

// @id is the key, @func is the function pointer
bool TaskEntityPool::FindToDo(const int64_t id, TaskFunc func) {
    ReadLocker locker(m_lock);
    // find the task
    map<int64_t, TaskPtr>::iterator it = m_task_map.find(id);
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
    for (map<int64_t, TaskPtr>::iterator it = m_task_map.begin();
         it != m_task_map.end(); ++it) {
        if ((it->second)->GetState() == TaskEntityState::TASKENTITY_WAITING) {
            if (false == (it->second)->Start()) {
                WriteLocker locker(m_lock);
                // task failed
                (it->second)->TaskFailed();
                LOG4CPLUS_ERROR(logger, "Fails to start the task, id:" << (it->second)->GetId());
            } else
                return;
        }
    }
}

// bind a function, then invoke it with FindToDo
// kill a taskEntity, and delete taskPtr from m_task_map
bool TaskEntityPool::KillTaskById(const int64_t id) {
    //TODO
    PrintAll();
    TaskPtr ptr = GetTaskPtr(id);

    // no the task
    if (!ptr) {
        LOG4CPLUS_ERROR(logger, "Can't find the task, fails to kill task, id:" << id);
        return false;
    }
     
    // fails to kill task
    if (!(ptr->Kill())) {
        LOG4CPLUS_ERROR(logger, "Fails to kill task, id:" << id);
        return false;
    }

    // delete task from pool(map)
    Delete(id);
    LOG4CPLUS_INFO(logger, "Kill task successfully, id:" << id);
    PrintAll();
    return true;
}
/*bool TaskEntityPool::DeleteTask(const int64_t id) {
    TaskFunc func = bind(&TaskEntity::Kill, _1);
    TaskPtr ptr = GetTaskPtr(id);
    if (FindToDo(id, func)) {
        Delete(id);
        return true;
    } else {
        return false;
    }
}*/

TaskPtr TaskEntityPool::GetTaskPtr(int64_t id) {
    ReadLocker locker(m_lock);
    map<int64_t, TaskPtr>::iterator it = m_task_map.find(id);
    if (it != m_task_map.end()) {
        return it->second;
    }
    // not find then return NULL
    return TaskPtr();
}
