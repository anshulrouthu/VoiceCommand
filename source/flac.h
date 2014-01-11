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


static void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data);

#define READSIZE 2048

static unsigned total_samples = 0; /* can use a 32-bit number due to WAVE size limitations */
static FLAC__int32 pcm[READSIZE/*samples*/ * 2/*channels*/];

class FLACWrapper
{
public:
    FLACWrapper(){};
    ~FLACWrapper();
    void init();
    void createFLAC(void* data);
    void deInit();
    void setParameters(int samples);
private:
    const char* c_str()
    {
        return ("WorkerThread");
    }
    FLAC__bool ok;
    FLAC__StreamEncoder *encoder;
    FLAC__StreamEncoderInitStatus init_status;
    FLAC__StreamMetadata *metadata[2];
    FLAC__StreamMetadata_VorbisComment_Entry entry;
    FILE *fin;
    unsigned sample_rate;
    unsigned channels;
    unsigned bps;
    int total_samples;

};


void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
    (void)encoder, (void)client_data;

    fprintf(stderr, "wrote %llu bytes, %llu/%u samples, %u/%u frames\n", bytes_written, samples_written, total_samples, frames_written, total_frames_estimate);
}

#endif /* FLAC_H_ */
