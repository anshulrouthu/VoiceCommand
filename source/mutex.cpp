/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * mutex.cpp
 *
 *  Created on: Feb 18, 2014
 *      Author: anshul
 */

#include "mutex.h"

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
 * Tries to lock the mutex, returns when the mutex is already locked
 * @return 0 if lock is acquired
 */
int Mutex::TryLock()
{
    return (pthread_mutex_trylock(&m_mutex));
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

/**
 * Constructor for Automutex
 */
AutoMutex::AutoMutex(Mutex* mutex) :
    m_mutex(mutex),
    m_locked(false)
{
    Lock();
}

/**
 * Destructor Automutex
 */
AutoMutex::~AutoMutex()
{
    Unlock();
}

/**
 * Lock the mutex, this is called in the constructor of this class
 */
VC_STATUS AutoMutex::Lock()
{
    if (m_mutex && !m_locked)
    {
        int err = m_mutex->Lock();
        VC_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Locking Mutex ", err);
        m_locked = true;
    }
    return (VC_SUCCESS);
}

/**
 * Unlock the mutex, this is called from destructor of the class
 */
VC_STATUS AutoMutex::Unlock()
{
    if (m_mutex && m_locked)
    {
        int err = m_mutex->Unlock();
        VC_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Unlocking Mutex ", err);
        m_locked = false;
    }
    return (VC_SUCCESS);
}

/**
 * Constructor for Automutexrelease
 */
AutoMutexRelease::AutoMutexRelease(Mutex* mutex) :
    m_mutex(mutex),
    m_locked(true)
{
    Unlock();
}

/**
 * Destructor for Automutexrelease
 */
AutoMutexRelease::~AutoMutexRelease()
{
    Lock();
}

/**
 * Locks the mutex, this is called from destructor
 */
VC_STATUS AutoMutexRelease::Lock()
{
    if (m_mutex && !m_locked)
    {
        int err = m_mutex->Lock();
        VC_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Locking Mutex ", err);
        m_locked = true;
    }
    return (VC_SUCCESS);
}

/**
 * Unlocks the mutex, this is called from constructor
 */
VC_STATUS AutoMutexRelease::Unlock()
{
    if (m_mutex && m_locked)
    {
        int err = m_mutex->Unlock();
        VC_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Unlocking Mutex ", err);
        m_locked = false;
    }
    return (VC_SUCCESS);
}

