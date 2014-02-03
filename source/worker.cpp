/*
 * worker.cpp
 *
 *  Created on: Oct 4, 2013
 *      Author: anshul
 */

#include "worker.h"

#ifndef WorkerThread_CPP_
#define WorkerThread_CPP_

WorkerThread::WorkerThread() :
    m_state(false), m_cv(m_mutex)
{
}

WorkerThread::~WorkerThread()
{
}

VC_STATUS WorkerThread::start()
{
    m_state = true;
    int t = pthread_create(&m_handle, NULL, &WorkerThread::taskLauncher, (void*) this);
    VC_CHECK(t != 0, return (VC_FAILURE), "Failed to create thread (error: %d)", t);

    return (VC_SUCCESS);

}

void* WorkerThread::taskLauncher(void* p)
{
    WorkerThread* c = static_cast<WorkerThread*>(p);
    c->Task();
    return (NULL);
}

int WorkerThread::join()
{
    return (pthread_join(m_handle, NULL));
}

void WorkerThread::stop()
{
    m_state = false;
}

Mutex::Mutex()
{
    pthread_mutex_init(&m_mutex,NULL);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_mutex);
}

int Mutex::Lock()
{
    return (pthread_mutex_lock(&m_mutex));
}

int Mutex::Unlock()
{
    return (pthread_mutex_unlock(&m_mutex));
}

ConditionVariable::ConditionVariable(Mutex& mutex) : m_mutex(mutex)
{
    pthread_cond_init(&m_condition,NULL);
}

ConditionVariable::~ConditionVariable()
{
    pthread_cond_destroy(&m_condition);
}

int ConditionVariable::Wait(int milliseconds)
{
    if(milliseconds == WAIT_FOREVER)
    {
        return (pthread_cond_wait(&m_condition,&m_mutex.m_mutex));
    }
    else
    {
        struct timeval  tv;
        struct timespec abstime;

        /* get the current time */
        gettimeofday( &tv, NULL );

        /* set our timeout */
        abstime.tv_sec  = tv.tv_sec + (milliseconds / 1000);
        abstime.tv_nsec = ( tv.tv_usec + ( (milliseconds % 1000) * 1000 ) ) * 1000;
        while ( abstime.tv_nsec >= NSEC_PER_SEC )
        {
            abstime.tv_sec  += 1;
            abstime.tv_nsec -= NSEC_PER_SEC;
        }

        return (pthread_cond_timedwait(&m_condition,&m_mutex.m_mutex, &abstime));

    }
}

int ConditionVariable::Notify()
{
    return (pthread_cond_signal(&m_condition));
}

#endif /* WorkerThread_CPP_ */
