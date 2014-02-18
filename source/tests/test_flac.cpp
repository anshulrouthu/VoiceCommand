/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * test_flac.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: anshul
 */

#include "utils.h"
#include "flac.h"
#include "apipe.h"

int main(int args, char* argv[])
{
    APipe* pipe = new APipe("Pipe0");
    ADevice* flac = pipe->GetDevice(VC_FLAC_DEVICE, "FlacTest");
    ADevice* cap = pipe->GetDevice(VC_CAPTURE_DEVICE, "AudioCapture");
    ADevice* sink = pipe->GetDevice(VC_FILESINK_DEVICE, "FileSink", "FileSink.out");
    flac->Initialize();
    cap->Initialize();
    sink->Initialize();
    pipe->ConnectDevices(cap, flac);
    pipe->ConnectDevices(flac, sink);

    cap->SendCommand(VC_CMD_START);
    flac->SendCommand(VC_CMD_START);
    sink->SendCommand(VC_CMD_START);

    while (getch() != 'q')
        ;

    cap->SendCommand(VC_CMD_STOP);
    flac->SendCommand(VC_CMD_STOP);
    sink->SendCommand(VC_CMD_STOP);

    pipe->DisconnectDevices(cap, flac);
    pipe->DisconnectDevices(flac, sink);

    delete cap;
    delete flac;
    delete sink;

    return (0);
}

