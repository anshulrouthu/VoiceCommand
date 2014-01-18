/*
 * timer.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */

#include <unistd.h>
#include "timer.h"

Timer::Timer(): m_start_time(0)
{
}

void Timer::StartTimer()
{
    m_start_time = clock();
}

void Timer::ResetTimer()
{
    m_start_time = clock();
}

int Timer::GetTimePassed()
{
    return ((clock()-m_start_time)/CLOCKS_PER_SEC);
}

