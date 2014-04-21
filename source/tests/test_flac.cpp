/*********************************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file test_flac.cpp is part of VoiceCommand project

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

