/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * test_curl.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: anshul
 */

#include "utils.h"
#include "apipe.h"
#include "file_io.h"
#include "curldevice.h"

int main(int argc, char* argv[])
{
    APipe* pipe = new APipe("Pipe");
    ADevice* curl = pipe->GetDevice(VC_CURL_DEVICE, "CURL Device");
    ADevice* filesrc = pipe->GetDevice(VC_FILESRC_DEVICE, "File Src", "FileSink.out");
    ADevice* filesink = pipe->GetDevice(VC_FILESINK_DEVICE, "File Sink", "curlout.txt");

    curl->Initialize();
    filesrc->Initialize();
    filesink->Initialize();

    pipe->ConnectDevices(filesrc, curl);
    pipe->ConnectDevices(curl, filesink);

    filesrc->SendCommand(VC_CMD_START);
    curl->SendCommand(VC_CMD_START);
    filesink->SendCommand(VC_CMD_START);

    while (getch() != 'q')
        ;

    curl->SendCommand(VC_CMD_STOP);
    filesrc->SendCommand(VC_CMD_STOP);
    filesink->SendCommand(VC_CMD_STOP);

    pipe->DisconnectDevices(filesrc, curl);
    pipe->DisconnectDevices(curl, filesink);

    delete curl;
    delete filesrc;
    delete filesink;
    delete pipe;

    return (0);
}

