/*
 * flac.cpp
 *
 *  Created on: Jan 10, 2014
 *      Author: anshul
 */
#include "flac.h"

void FLACWrapper::init()
{
    ok = true;
    sample_rate = 0;
    channels = 0;
    bps = 0;

}

FLACWrapper::~FLACWrapper()
{
    ok &= FLAC__stream_encoder_finish(encoder);

    fprintf(stderr, "encoding: %s\n", ok? "succeeded" : "FAILED");
    fprintf(stderr, "   state: %s\n", FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);

    /* now that encoding is finished, the metadata can be freed */
    FLAC__metadata_object_delete(metadata[0]);
    FLAC__metadata_object_delete(metadata[1]);

    FLAC__stream_encoder_delete(encoder);
}
void FLACWrapper::setParameters(int samples)
{
    sample_rate = 16000;
    channels = 2;
    bps = 16;
    total_samples = samples;

    printf("sample_rate: %d %d\n",sample_rate,total_samples);

    /* allocate the encoder */
    if((encoder = FLAC__stream_encoder_new()) == NULL) {
        fprintf(stderr, "ERROR: allocating encoder\n");
        fclose(fin);
        return;
    }

    ok &= FLAC__stream_encoder_set_verify(encoder, true);
    ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
    ok &= FLAC__stream_encoder_set_channels(encoder, channels);
    ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bps);
    ok &= FLAC__stream_encoder_set_sample_rate(encoder, sample_rate);
    ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, total_samples);

    /* now add some metadata; we'll add some tags and a padding block */
    if(ok) {
        if(
            (metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) == NULL ||
            (metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL ||
            /* there are many tag (vorbiscomment) functions but these are convenient for this particular use: */
            !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", "Some Artist") ||
            !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) || /* copy=false: let metadata object take control of entry's allocated string */
            !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "YEAR", "1984") ||
            !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false)
        ) {
            fprintf(stderr, "ERROR: out of memory or tag error\n");
            ok = false;
        }

        metadata[1]->length = 1234; /* set the padding length */

        ok = FLAC__stream_encoder_set_metadata(encoder, metadata, 2);
    }

    /* initialize encoder */
    if(ok) {
        init_status = FLAC__stream_encoder_init_file(encoder, "audio.flac", progress_callback, /*client_data=*/NULL);
        if(init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
            fprintf(stderr, "ERROR: initializing encoder: %s\n", FLAC__StreamEncoderInitStatusString[init_status]);
            ok = false;
        }
    }
}

void FLACWrapper::createFLAC(void* data)
{
    /* read blocks of samples from WAVE file and feed to encoder */
    FLAC__byte* buffer;
    if(ok) {
        buffer = (FLAC__byte*)data;
        size_t left = (size_t)total_samples;
        while(ok && left) {
            size_t need = (left>READSIZE? (size_t)READSIZE : (size_t)left);
            {
                /* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
                size_t i;
                for(i = 0; i < need*channels; i++) {
                    /* inefficient but simple and works on big- or little-endian machines */
                    pcm[i] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)buffer[2*i+1] << 8) | (FLAC__int16)buffer[2*i]);
                }
                /* feed samples to encoder */
                ok = FLAC__stream_encoder_process_interleaved(encoder, pcm, need);
            }

            left -= need;
            buffer+=need*4;
        }

    }
}

