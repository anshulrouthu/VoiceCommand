/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
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
#define SAMPLE_RATE 16000
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
    VC_STATUS InitiateFLACCapture();
    VC_STATUS CloseFLACCapture();
private:
    static void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written,
        FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data);
    static FLAC__StreamEncoderWriteStatus write_callback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[],
        size_t bytes, unsigned samples, unsigned current_frame, void *client_data);

    const char* c_str()
    {
        return (m_name.c_str());
    }

    FLAC__StreamEncoder* m_encoder;
    FLAC__StreamMetadata* m_metadata[2];
    const char* m_filename;
    std::string m_name;
    ClientData m_cdata;
    InputPort* m_input;
    OutputPort* m_output;
};

#endif /* FLAC_H_ */
