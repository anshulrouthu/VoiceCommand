/*********************************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file test_curl.cpp is part of VoiceCommand project

VoiceCommand is a free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

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
    ADevice* filesrc = pipe->GetDevice(VC_FILESRC_DEVICE, "File Src", argv[1]);
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

