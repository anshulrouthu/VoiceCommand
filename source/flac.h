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
    static void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data);
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
    FLAC__int32 pcm[READSIZE/*samples*/ * 2/*channels*/];

};

#endif /* FLAC_H_ */
