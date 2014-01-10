/*
 * timer.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */

#include <unistd.h>
#include "timer.h"

Timer::Timer(): start_time(0),time_passed(0),m_running(false)
{
    start();
}

void Timer::StartTimer()
{
    m_running = true;
    start_time = clock();
}

void Timer::ResetTimer()
{
    m_running = false;
    start_time = 0;
    time_passed = 0;
}

int Timer::GetTimePassed()
{
    return ((time_passed-start_time)/CLOCKS_PER_SEC);
}

void Timer::Task()
{
    while(m_state)
    {
        if(m_running)
        {
            time_passed = clock();
        }
        //usleep(1000);
    }
}

