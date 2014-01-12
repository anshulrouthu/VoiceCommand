/*
 * voiceCommand.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */
#include "utils.h"
#include "timer.h"
#include "aldevice.h"
#include "voiceCommand.h"
#include "flac.h"

int main(int argc, char* argv[])
{
    int c;
    while ((c = getopt (argc, argv, "?:d:")) != -1)
    {
        switch (c)
        {
        case 'd':
            g_dbglevel = (int)strtol(optarg,NULL,0);
            break;
        default:
            break;
        }
    }

    DBG_PRINT(DBG_ALWAYS,"Debug Level %d",g_dbglevel);
    Timer* timer = new Timer();
    ALDevice* device= new ALDevice();
    FLACWrapper* flac = new FLACWrapper();
    flac->init();
    device->init();
    device->startCapture();
    timer->StartTimer();
    while(!kbhit());
    device->stopCapture();
    timer->ResetTimer();
    device->createWAV();
    flac->setParameters(device->getNoSamples());
    writeWAVData("audio.wav", (ALshort*) device->getData(), device->getNoSamples() * 2, 16000, 2);
    flac->createFLAC(device->getData());
    delete flac;
    delete device;
    delete timer;
    return(0);
}



