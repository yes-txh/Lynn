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
    Event(EventType::type type) : m_type(type) {}
    virtual ~Event() {}
    virtual bool Handle() = 0;
    EventType::type GetType() const {
        return m_type;
    }

private:
    EventType::type m_type;
}; 

class StateEvent : public Event {
public:
    explicit StateEvent(TaskID id, bool state)
        : Event(EventType::STATE_EVENT), m_id(id), m_state(state) {}
    virtual ~StateEvent() {}

    TaskID GetID() const {
        return m_id;
    }

    bool GetState() const {
        return m_state;
    }

private:
    TaskID m_id;
    // TODO?
    bool m_state;
};

class ActionEvent : public Event {
public:
    explicit ActionEvent(TaskID id) : Event(EventType::ACTION_EVENT), m_id(id) {}
    virtual ~ActionEvent() {} 

    TaskID GetID() const {
        return m_id;
    }
private:
    TaskID m_id;    
};

class KillActionEvent : public ActionEvent {
public:
    KillActionEvent(TaskID id) : ActionEvent(id) {}
    bool Handle();
};

class InstallAppEvent : public ActionEvent {
public:
    InstallAppEvent(TaskID id) : ActionEvent(id) {}
    bool Handle();
}; 

typedef shared_ptr<Event> EventPtr;
typedef BlockQueue<EventPtr> EventQueue;

#endif
