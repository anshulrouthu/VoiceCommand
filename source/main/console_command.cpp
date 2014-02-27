/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * voiceCommand.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */
#include "utils.h"
#include "timer.h"
#include "capturedevice.h"
#include "command_processor.h"
#include "console_command.h"
#include "flac.h"
#include "apipe.h"

int main(int argc, char* argv[])
{
    int c;
    int threshold = 1500;
    bool autosetup = false, filecapture = false;
    ADevice* src;
    ADevice* sink;
    while ((c = getopt(argc, argv, "sf?l:d:t:")) != -1)
    {
        switch (c)
        {
        case 'd':
            DebugSetLevel(strtol(optarg, NULL, 10));
            break;
        case 'l':
            //device->GetCaptureDeviceList();
            exit(0);
            break;
        case 's':
            autosetup = true;
            break;
        case 't':
            threshold = (int) strtol(optarg, NULL, 10);
            DBG_PRINT(DBG_TRACE, "Audio Volume threshold: %d", threshold);
            break;
        case 'f':
            filecapture = true;
            break;
        default:
            break;
        }
    }

    APipe* pipe = new APipe("Pipe 0");
    src = pipe->GetDevice(VC_CAPTURE_DEVICE, "CaptureDevice 0");

    if (filecapture)
    {
        sink = pipe->GetDevice(VC_FILESINK_DEVICE, "FileCapture", "FileSink.out");
    }
    else
    {
        sink = pipe->GetDevice(VC_AUDIO_PROCESSOR, "AudioProcessor 0");
    }

    src->Initialize();
    sink->Initialize();
    InputParams params;
    params.threshold = threshold;
    src->SetParameters(&params);

    pipe->ConnectDevices(src, sink);
    src->SendCommand(VC_CMD_START);
    sink->SendCommand(VC_CMD_START);

    while (c != 'q')
    {
        DBG_PRINT(DBG_TRACE, "Please hit 'q' to exit");
        c = getch();
        DBG_PRINT(DBG_TRACE, "key hit %c", c);

        usleep(10000);
    }

    src->SendCommand(VC_CMD_STOP);
    sink->SendCommand(VC_CMD_STOP);
    pipe->DisconnectDevices(src, sink);

    delete src;
    delete sink;

    return (0);
}

