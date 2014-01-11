/*
 * worker.h
 *
 *  Created on: Oct 4, 2013
 *      Author: anshul
 */

#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include<stdio.h>
#include<pthread.h>
#include "utils.h"

class WorkerThread
{
public:
    WorkerThread();
    virtual ~WorkerThread();
    void start();
    void join();
private:
    virtual const char* c_str()
    {
        return ("WorkerThread");
    }
    virtual void Task()=0;
    static void* taskLauncher(void* args);
    pthread_t m_handle;

protected:
    bool m_state;
};

#endif /* WORKER_THREAD_H_ */
