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
#include<sys/time.h>
#include "utils.h"

#define WAIT_FOREVER -1
static int const NSEC_PER_SEC = 1000000000;

class ConditionVariable;

class Mutex
{
    friend class ConditionVariable;
public:
    Mutex();
    ~Mutex();
    int Lock();
    int Unlock();
private:
    pthread_mutex_t m_mutex;

};

class ConditionVariable
{
public:
    ConditionVariable(Mutex& mutex);
    ~ConditionVariable();
    int Notify();
    int Wait(int millisconds = WAIT_FOREVER);
private:
    pthread_cond_t  m_condition;
    Mutex& m_mutex;
};

class WorkerThread
{
public:
    WorkerThread();
    virtual ~WorkerThread();
    VC_STATUS start();
    int join();
    void stop();
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
    Mutex m_mutex;
    ConditionVariable m_cv;
};

#endif /* WORKER_THREAD_H_ */
