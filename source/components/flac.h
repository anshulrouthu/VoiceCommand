/***********************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file flac.h is part of VoiceCommand project

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
***********************************************************/

/*
 * flac.h
 *
 *  Created on: Jan 10, 2014
 *      Author: anshul
 */

#ifndef FLAC_H_
#define FLAC_H_

#include "utils.h"
#include <ctime>
#include "FLAC/metadata.h"
#include "FLAC/stream_encoder.h"
#include "apipe.h"

typedef struct client_data
{
    int samples;
} ClientData;

#define READSIZE 512
#define ENCODING_SAMPLE_RATE 44100
#define BITS_PER_SECOND 16

class FLACDevice: public ADevice
{
public:
    FLACDevice(std::string name, const char* filename = "audio.flac");
    virtual ~FLACDevice();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual InputPort* Input(int portno);
    virtual OutputPort* Output(int portno);
    virtual VC_STATUS SendCommand(VC_CMD cmd);

    int WriteData(void* data, int total_samples);
    VC_STATUS setParameters();
    VC_STATUS StartEncoder();
    VC_STATUS StopEncoder();
private:
    static void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written,
        FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data);
    static FLAC__StreamEncoderWriteStatus write_callback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[],
        size_t bytes, unsigned samples, unsigned current_frame, void *client_data);

    FLAC__StreamEncoder* m_encoder;
    FLAC__StreamMetadata* m_metadata[2];
    const char* m_filename;
    ClientData m_cdata;
    InputPort* m_input;
    OutputPort* m_output;
    bool m_ready;
};

#endif /* FLAC_H_ */
