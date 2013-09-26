/********************************
 FileName: executor/event.h
 Author:   WangMin
 Date:     2013-09-24
 Version:  0.1
 Description: event, and its handler
*********************************/

#ifndef SRC_EXECUTOR_EVENT_H
#define SRC_EXECUTOR_EVENT_H

#include <string>
#include <boost/shared_ptr.hpp>

#include "common/clynn/block_queue.h"

#include "include/proxy.h"
#include "executor/type.h"

using std::string;
using boost::shared_ptr;
using clynn::BlockQueue;

class Event {
public:
    Event(EventType::type type, TaskID id) : m_type(type), m_id(id) {}
    virtual ~Event() {}
    virtual bool Handle() = 0;
    EventType::type GetType() const {
        return m_type;
    }

    TaskID GetID() const {
        return m_id;
    }

private:
    TaskID m_id;
    EventType::type m_type;
}; 

class StateEvent : public Event {
public:
    explicit StateEvent(TaskID id, bool state)
        : Event(EventType::STATE_EVENT, id), m_state(state) {}
    virtual ~StateEvent() {}

    bool GetState() const {
        return m_state;
    }

private:
    // TODO?
    bool m_state;
};

class ActionEvent : public Event {
public:
    explicit ActionEvent(TaskID id) : Event(EventType::ACTION_EVENT, id) {}
    virtual ~ActionEvent() {} 
};

class TaskEvent : public Event {
public:
    explicit TaskEvent(TaskID id) : Event(EventType::TASK_EVENT, id) {}
    virtual ~TaskEvent() {} 
};

class StopActionEvent : public ActionEvent {
public:
    StopActionEvent(TaskID id) : ActionEvent(id) {}
    bool Handle();
};

class KillActionEvent : public ActionEvent {
public:
    KillActionEvent(TaskID id) : ActionEvent(id) {}
    bool Handle();
};

class InstallAppEvent : public TaskEvent {
public:
    InstallAppEvent(TaskID id) : TaskEvent(id) {}
    bool Handle();
}; 

class StartAppEvent : public TaskEvent {
public:
    StartAppEvent(TaskID id) : TaskEvent(id) {}
    bool Handle();
};

typedef shared_ptr<Event> EventPtr;
typedef BlockQueue<EventPtr> EventQueue;

#endif
