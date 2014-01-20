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
#include <fstream>

#define READSIZE 2048
#define SAMPLE_RATE 16000
#define NO_OF_CHANNELS 2
#define BITS_PER_SECOND 16

class FLACWrapper
{
public:
    FLACWrapper(char* filename);
    ~FLACWrapper();
    VC_STATUS createFLAC(void* data, int total_samples);
    VC_STATUS setParameters();
private:
    static void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data);
    const char* c_str()
    {
        return ("FLACWrapper");
    }

    FLAC__StreamEncoder* m_encoder;
    FLAC__StreamMetadata* m_metadata[2];
    char* m_filename;
};

#endif /* FLAC_H_ */
