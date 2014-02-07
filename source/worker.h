/*
 * worker.h
 *
 *  Created on: Oct 4, 2013
 *      Author: anshul
 */

/**
 * @file worker.h
 *
 * Main interface for all the threads, mutexs and condition variables in the application.
 *
 */
#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include<pthread.h>
#include<sys/time.h>
#include "utils.h"

#define WAIT_FOREVER -1
static int const NSEC_PER_SEC = 1000000000;

class ConditionVariable;

/**
 *  A wrapper object for pthread mutex. that allows easy locking and unlocking
 */
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

/**
 * A wrapper object for pthread condition variables. that allows easy wait and signal event
 */
class ConditionVariable
{
public:
    ConditionVariable(Mutex& mutex);
    ~ConditionVariable();
    int Notify();
    int Wait(int millisconds = WAIT_FOREVER);
private:
    pthread_cond_t m_condition;
    Mutex& m_mutex;
};

/**
 * An interface for all the threads in the application.
 */
class WorkerThread
{
public:
    WorkerThread();
    virtual ~WorkerThread();
    VC_STATUS Start();
    int Join();
    void Stop();
private:
    const char* c_str()
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
