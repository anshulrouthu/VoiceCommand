/*********************************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file osapi_linux.cpp is part of VoiceCommand project

VoiceCommand is a free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "osapi_linux.h"

int OS_MUTEX_INIT(OS_MUTEX* mutex, OS_MUTEX_ATTR* attr)
{
    return (pthread_mutex_init(mutex, attr));
}

int OS_MUTEX_DESTROY(OS_MUTEX* mutex)
{
    return (pthread_mutex_destroy(mutex));
}

int OS_MUTEX_LOCK(OS_MUTEX* mutex)
{
    return (pthread_mutex_lock(mutex));
}

int OS_MUTEX_UNLOCK(OS_MUTEX* mutex)
{
    return (pthread_mutex_unlock(mutex));
}

int OS_MUTEX_TRYLOCK(OS_MUTEX* mutex)
{
    return (pthread_mutex_trylock(mutex));
}

int OS_COND_INIT(OS_COND* cond, OS_COND_ATTR* attr)
{
    return (pthread_cond_init(cond, attr));
}

int OS_COND_DESTROY(OS_COND* cond)
{
    return (pthread_cond_destroy(cond));
}

int OS_COND_WAIT(OS_COND* cond, OS_MUTEX* mutex)
{
    return (pthread_cond_wait(cond, mutex));
}

int OS_COND_TIMEDWAIT(OS_COND* cond, OS_MUTEX* mutex, struct timespec* abstime)
{
    return (pthread_cond_timedwait(cond, mutex, abstime));
}

int OS_COND_SIGNAL(OS_COND* cond)
{
    return (pthread_cond_signal(cond));
}

int OS_THREAD_CREATE(OS_THREAD_HANDLE* handle, OS_THREAD_ATTR* attr, void *(*start_routine)(void *), void* appdata)
{
    return (pthread_create(handle, attr, start_routine, appdata));
}

int OS_THREAD_JOIN(OS_THREAD_HANDLE* handle, void** attr)
{
    return (pthread_join(*handle, attr));
}
