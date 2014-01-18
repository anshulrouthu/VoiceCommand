/*
 * timer.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */

#include <unistd.h>
#include "timer.h"

Timer::Timer(): m_start_time(0),m_time_passed(0),m_running(false)
{
    start();
}

void Timer::StartTimer()
{
    m_running = true;
    m_start_time = clock();
}

void Timer::ResetTimer()
{
    //m_running = false;
    m_start_time = m_time_passed = clock();
}

int Timer::GetTimePassed()
{
    return ((m_time_passed-m_start_time)/CLOCKS_PER_SEC);
}

void Timer::Task()
{
    while(m_state)
    {
        if(m_running)
        {
            m_time_passed = clock();
        }
        //usleep(1000);
    }
}

