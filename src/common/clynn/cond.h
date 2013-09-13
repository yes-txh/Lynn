/********************************
 FileName: common/clynn/cond.h
 Author:   
 Date:     2013-08-21
 Version:  0.1
 Description: pthread condition lock 
*********************************/

#ifndef COMMON_COND_H
#define COMMON_COND_H

#include <pthread.h>

#include "common/clynn/mutex.h"

namespace clynn {

    class Cond {
        public:
            Cond();
            ~Cond();

            /// send signal to wake up the thread
            void Signal();

            /// block the thread, waitting to be woken up
            void Wait(Mutex& mutex);

            /// block the thread with timeout
            int Wait(Mutex& mutex, size_t timeout);

        private:
            void CheckError(const char* info, int code);

        private:
            pthread_cond_t m_cond;
    };
}

#endif
