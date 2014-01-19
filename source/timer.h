/*
 * timer.h
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */

#ifndef TIMER_H_
#define TIMER_H_

#include<time.h>
#include<stdio.h>
#include "worker.h"

class Timer
{
public:
    Timer();
    virtual ~Timer(){}
    void StartTimer();
    void ResetTimer();
    float GetTimePassed();

private:
    virtual const char* c_str()
    {
        return ("Timer");
    }
    clock_t m_start_time;
};



#endif /* TIMER_H_ */
