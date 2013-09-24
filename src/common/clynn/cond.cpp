/********************************
 FileName: common/clynn/cond.cpp
 Author:   
 Date:     2013-08-21
 Version:  0.1
 Description: pthread condition lock 
*********************************/

#include <string>
#include <stdexcept>
#include <string.h>
#include "common/clynn/cond.h"

using std::string;
using std::runtime_error;

clynn::Cond::Cond() {
    CheckError("Cond::Cond", pthread_cond_init(&m_cond, NULL));
}

clynn::Cond::~Cond() {
    pthread_cond_destroy(&m_cond);
}

void clynn::Cond::Signal() {
    CheckError("Cond::Signal", pthread_cond_signal(&m_cond));
}

void clynn::Cond::Wait(Mutex& mutex) {
    CheckError("Cond::Wait", pthread_cond_wait(&m_cond, &(mutex.m_lock)));
}

int clynn::Cond::Wait(Mutex& mutex, size_t timeout) {
    struct timespec time;
    time.tv_sec = timeout;
    time.tv_nsec = 0;
    int ret = pthread_cond_timedwait(&m_cond, &(mutex.m_lock), &time);
    CheckError("Cond::TimeWait", ret);
    return ret;
}

void clynn::Cond::CheckError(const char* info, int code) {
    if (code != 0) {
        string msg = info;
        msg += " error: ";
        msg += strerror(code);
        throw runtime_error(msg);
    }
}

