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

class Timer: public WorkerThread
{
public:
    Timer();
    virtual ~Timer(){}
    void StartTimer();
    void ResetTimer();
    int GetTimePassed();

private:
    virtual const char* c_str()
    {
        return ("Timer");
    }
    virtual void Task();
    clock_t start_time;
    clock_t time_passed;
    bool m_running;
};



#endif /* TIMER_H_ */
