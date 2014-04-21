/*********************************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file test_osapi.cpp is part of VoiceCommand project

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

/*
 * test_osapi.cpp
 *
 *  Created on: Mar 8, 2014
 *      Author: anshul
 */

#include "utils.h"
#include "osapi.h"
#include "mutex.h"

Mutex mutex;
ConditionVariable cv(mutex);
bool wakeup = false;

static void* ThreadTest(void* p)
{
    AutoMutex automutex(&mutex);
    DBGPRINT(DBG_ALWAYS, ("Mutex held by %s\n", __FUNCTION__));
    while (!wakeup)
    {
        DBGPRINT(DBG_ALWAYS, ("%s is sleeping\n", __FUNCTION__));
        cv.Wait();
    }
    return 0;
}

int main(int argc, char* argv[])
{
    DBGPRINT(DBG_ALWAYS, ("OSAPI Component Test\n"));

    OS_THREAD_HANDLE handle;

    OS_THREAD_CREATE(&handle, NULL, &ThreadTest, NULL );
    usleep(100000);
    {
        AutoMutex automutex(&mutex);
        DBGPRINT(DBG_ALWAYS, ("Mutex held by %s\n", __FUNCTION__));
        wakeup = true;
        DBGPRINT(DBG_ALWAYS, ("Wake up the thread\n"));
        cv.Notify();
    }

    OS_THREAD_JOIN(&handle, NULL );

    DBGPRINT(DBG_ALWAYS, ("OSAPI Component Test Successful\n"));
    return (0);
}
