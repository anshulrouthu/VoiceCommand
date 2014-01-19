/*
 * voiceCommand.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */
#include "utils.h"
#include "timer.h"
#include "aldevice.h"
#include "command_processor.h"
#include "console_command.h"
#include "flac.h"

int main(int argc, char* argv[])
{
    ALDevice* device= new ALDevice();

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
            exit(0);
            break;
        default:
            break;
        }
    }

    device->StartCapture();
    while(c!='q')
    {
        c = getch();
        DBG_PRINT(DBG_TRACE,"key hit %c",c);

        usleep(10000);
    }
    //device->Init();
    //timer->StartTimer();
    //while(!kbhit());
    device->StopCapture();
    device->join();
    //device->StopCapture();
    //timer->ResetTimer();
    //device->CreateWAV();
    //writeWAVData("audio.wav", (ALshort*) device->GetData(), device->GetNoSamples() * 2, 16000, 2);
    delete device;

    return(0);
}



