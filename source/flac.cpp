/*
 * flac.cpp
 *
 *  Created on: Jan 10, 2014
 *      Author: anshul
 */
#include "flac.h"

FLACWrapper::FLACWrapper()
{
   m_encoder = FLAC__stream_encoder_new();
   VC_CHECK(m_encoder == NULL,,"Error Initializing FLAC encoder");
}

FLACWrapper::~FLACWrapper()
{
    FLAC__metadata_object_delete(m_metadata[0]);
    FLAC__metadata_object_delete(m_metadata[1]);
    FLAC__stream_encoder_delete(m_encoder);
}

VC_STATUS FLACWrapper::setParameters()
{
    bool ok = true;
    FLAC__StreamMetadata_VorbisComment_Entry entry;

    ok &= FLAC__stream_encoder_set_verify(m_encoder, true);
    ok &= FLAC__stream_encoder_set_compression_level(m_encoder, 5);
    ok &= FLAC__stream_encoder_set_channels(m_encoder, NO_OF_CHANNELS);
    ok &= FLAC__stream_encoder_set_bits_per_sample(m_encoder, BITS_PER_SECOND);
    ok &= FLAC__stream_encoder_set_sample_rate(m_encoder, SAMPLE_RATE);

    VC_CHECK(!ok, return (VC_FAILURE), "Failed to set FLAC parameters");

    if (ok)
    {
        if ((m_metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) == NULL || (m_metadata[1] =
            FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL ||
        !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", "voiceCommand")
            || !FLAC__metadata_object_vorbiscomment_append_comment(m_metadata[0], entry, /*copy=*/false) || /* copy=false: let metadata object take control of entry's allocated string */
            !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "YEAR", "2014")
            || !FLAC__metadata_object_vorbiscomment_append_comment(m_metadata[0], entry, /*copy=*/false))
        {
            VC_ERR("ERROR: out of memory or tag error");
            return (VC_FAILURE);
        }

        m_metadata[1]->length = 1234; /* set the padding length */

        FLAC__stream_encoder_set_metadata(m_encoder, m_metadata, 2);
    }
    return (VC_SUCCESS);
}

VC_STATUS FLACWrapper::createFLAC(void* data, int total_samples)
{
    FLAC__byte* buffer;
    buffer = (FLAC__byte*) data;
    size_t left = (size_t) total_samples;
    FLAC__int32 pcm[READSIZE * 2];
    FLAC__StreamEncoderInitStatus init_status;

    setParameters();
    init_status = FLAC__stream_encoder_init_file(m_encoder, "audio.flac", FLACWrapper::progress_callback, NULL);
    VC_CHECK(init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK, return (VC_FAILURE), "ERROR: initializing encoder: %s",FLAC__StreamEncoderInitStatusString[init_status]);

    while (left)
    {
        size_t need = (left > READSIZE ? (size_t) READSIZE : (size_t) left);
        {
            size_t i;
            for (i = 0; i < need * NO_OF_CHANNELS; i++)
            {
                pcm[i] = (FLAC__int32) (((FLAC__int16) (FLAC__int8) buffer[2 * i + 1] << 8) | (FLAC__int16) buffer[2 * i]);
            }
            FLAC__stream_encoder_process_interleaved(m_encoder, pcm, need);
        }
        left -= need;
        buffer += need * 4;
    }

    FLAC__stream_encoder_finish(m_encoder);

    return (VC_SUCCESS);
}

void FLACWrapper::progress_callback(const FLAC__StreamEncoder *m_encoder, FLAC__uint64 bytes_written,
    FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
    (void) m_encoder, (void) client_data;

    //fprintf(stderr, "wrote %llu bytes, %llu samples, %u/%u frames", bytes_written, samples_written, frames_written, total_frames_estimate);
}

