/*
 * worker.cpp
 *
 *  Created on: Oct 4, 2013
 *      Author: anshul
 */
/**
 * @file worker.cpp
 *
 * Main interface for all the thread in the application.
 *
 */
#include "worker.h"

#ifndef WORKERTHREAD_CPP_
#define WORKERTHREAD_CPP_

/**
 * Workerthread Constructor
 */
WorkerThread::WorkerThread() :
    m_state(false)
{
}

/**
 * Workerthread Destructor
 */
WorkerThread::~WorkerThread()
{
}

/**
 * This function creates a thread of calling instance
 */
VC_STATUS WorkerThread::Start()
{
    m_state = true;
    int t = pthread_create(&m_handle, NULL, &WorkerThread::taskLauncher, (void*) this);
    VC_CHECK(t != 0, return (VC_FAILURE), "Failed to create thread (error: %d)", t);

    return (VC_SUCCESS);

}

/**
 * A static function that initiates the calling object's thread.
 */
void* WorkerThread::taskLauncher(void* p)
{
    WorkerThread* c = static_cast<WorkerThread*>(p);
    c->Task();
    return (NULL);
}

/**
 * Joins the thread
 */
int WorkerThread::Join()
{
    return (pthread_join(m_handle, NULL));
}

/**
 * Stops the thread
 */
void WorkerThread::Stop()
{
    m_state = false;
}

/**
 * mutex Constructor
 */
Mutex::Mutex()
{
    pthread_mutex_init(&m_mutex, NULL);
}

/**
 * mutex Destructor
 */
Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_mutex);
}

/**
 * Locks the mutex
 * @return 0 on success or error code
 */
int Mutex::Lock()
{
    return (pthread_mutex_lock(&m_mutex));
}

/**
 * Unlocks the mutex
 * @return 0 on success or error code
 */
int Mutex::Unlock()
{
    return (pthread_mutex_unlock(&m_mutex));
}

/**
 * Conditionvariable Constructor
 * @param [in] mutex
 */
ConditionVariable::ConditionVariable(Mutex& mutex) :
    m_mutex(mutex)
{
    pthread_cond_init(&m_condition, NULL);
}

/**
 * Conditionvariable Destructor
 */
ConditionVariable::~ConditionVariable()
{
    pthread_cond_destroy(&m_condition);
}

/**
 * Waits for the condition to signal or till timeout
 * @param[in] milliseconds timeout in ms (If no value is provided waits till the signal)
 * @return 0 on success or error code
 */
int ConditionVariable::Wait(int milliseconds)
{
    if (milliseconds == WAIT_FOREVER)
    {
        return (pthread_cond_wait(&m_condition, &m_mutex.m_mutex));
    }
    else
    {
        struct timeval tv;
        struct timespec abstime;

        /* get the current time */
        gettimeofday(&tv, NULL);

        /* set our timeout */
        abstime.tv_sec = tv.tv_sec + (milliseconds / 1000);
        abstime.tv_nsec = (tv.tv_usec + ((milliseconds % 1000) * 1000)) * 1000;
        while (abstime.tv_nsec >= NSEC_PER_SEC)
        {
            abstime.tv_sec += 1;
            abstime.tv_nsec -= NSEC_PER_SEC;
        }

        return (pthread_cond_timedwait(&m_condition, &m_mutex.m_mutex, &abstime));
    }
}

/**
 * Signals the Conditionvariable that is on wait
 * @return 0 on success or error code
 */
int ConditionVariable::Notify()
{
    return (pthread_cond_signal(&m_condition));
}

#endif /* WORKERTHREAD_CPP_ */
