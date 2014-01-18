/*
 * worker.cpp
 *
 *  Created on: Oct 4, 2013
 *      Author: anshul
 */

#include "worker.h"

#ifndef WorkerThread_CPP_
#define WorkerThread_CPP_


WorkerThread::WorkerThread():m_state(false)
{

}
WorkerThread::~WorkerThread()
{
}

void WorkerThread::start()
{
    m_state = true;
    int t = pthread_create(&m_handle,NULL,&WorkerThread::taskLauncher,(void*)this);
    VC_CHECK(t!=0,exit(1),"Failed to create thread (error: %d)",t);

}

void* WorkerThread::taskLauncher(void* p)
{
    WorkerThread* c = static_cast<WorkerThread*>(p);
    c->Task();
    return (NULL);
}
void WorkerThread::join()
{
    pthread_join(m_handle,NULL);
}

void WorkerThread::stop()
{
    m_state = false;
}

#endif /* WorkerThread_CPP_ */
