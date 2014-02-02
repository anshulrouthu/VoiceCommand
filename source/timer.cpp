/*
 * timer.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */

#include <unistd.h>
#include "timer.h"

Timer::Timer()
{
}

void Timer::StartTimer()
{
    gettimeofday(&m_start_time, NULL);
}

void Timer::ResetTimer()
{
    gettimeofday(&m_start_time, NULL);
}

long Timer::GetTimePassed()
{
    long mtime, seconds, useconds;

    gettimeofday(&m_end_time, NULL);
    seconds  = m_end_time.tv_sec  - m_start_time.tv_sec;
    useconds = m_end_time.tv_usec - m_start_time.tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

    return (mtime);
}

long Timer::StopTimer()
{
    long mtime, seconds, useconds;

    gettimeofday(&m_end_time, NULL);
    seconds  = m_end_time.tv_sec  - m_start_time.tv_sec;
    useconds = m_end_time.tv_usec - m_start_time.tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

    return (mtime);
}
