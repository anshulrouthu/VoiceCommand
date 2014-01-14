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
    Timer* timer = new Timer();
    ALDevice* device= new ALDevice();
    FLACWrapper* flac = new FLACWrapper();

    int c;
    while ((c = getopt (argc, argv, "?l:d:")) != -1)
    {
        switch (c)
        {
        case 'd':
            DebugSetLevel(strtol(optarg,NULL,10));
            break;
        case 'l':
            device->GetCaptureDeviceList();
            return (0);
            break;
        default:
            break;
        }
    }

    flac->init();
    device->Init();
    device->StartCapture();
    timer->StartTimer();
    while(!kbhit());
    device->StopCapture();
    timer->ResetTimer();
    device->CreateWAV();
    flac->setParameters(device->GetNoSamples());
    writeWAVData("audio.wav", (ALshort*) device->GetData(), device->GetNoSamples() * 2, 16000, 2);
    flac->createFLAC(device->GetData(),device->GetNoSamples());
    delete flac;
    delete device;
    delete timer;
    return(0);
}



