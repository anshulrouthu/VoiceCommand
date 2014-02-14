/***********************************************************
voiceCommand 

  Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

  All rights reserved.

  This software is distributed on an "AS IS" BASIS, 
  WITHOUT  WARRANTIES OR CONDITIONS OF ANY KIND, either 
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

typedef struct client_data
{
    int samples;
}ClientData;


#define READSIZE 512
#define SAMPLE_RATE 16000
#define BITS_PER_SECOND 16

class FLACWrapper
{
public:
    FLACWrapper(char* filename);
    ~FLACWrapper();
    int WriteData(void* data, int total_samples);
    VC_STATUS setParameters();
    VC_STATUS InitiateFLACCapture();
    VC_STATUS CloseFLACCapture();
private:
    static void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data);
    const char* c_str()
    {
        return ("FLACWrapper");
    }

    FLAC__StreamEncoder* m_encoder;
    FLAC__StreamMetadata* m_metadata[2];
    char* m_filename;
    ClientData m_cdata;
};

#endif /* FLAC_H_ */
