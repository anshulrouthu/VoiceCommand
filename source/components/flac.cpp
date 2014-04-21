/*********************************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file flac.cpp is part of VoiceCommand project

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
 * flac.cpp
 *
 *  Created on: Jan 10, 2014
 *      Author: anshul
 */
#include "flac.h"

FLACDevice::FLACDevice(std::string name, const char* filename) :
    ADevice(name),
    m_filename(filename),
    m_ready(false)
{
    m_encoder = FLAC__stream_encoder_new();
    VC_CHECK(m_encoder == NULL,, "Error Initializing FLAC encoder");
    setParameters();
}

FLACDevice::~FLACDevice()
{
    FLAC__metadata_object_delete(m_metadata[0]);
    FLAC__metadata_object_delete(m_metadata[1]);
    FLAC__stream_encoder_delete(m_encoder);
    delete m_input;
    delete m_output;
}

VC_STATUS FLACDevice::Initialize()
{
    m_input = new InputPort("FLACInput 0", this);
    m_output = new OutputPort("FLACOutput 0", this);

    return (VC_SUCCESS);
}

VC_STATUS FLACDevice::SendCommand(VC_CMD cmd)
{
    switch (cmd)
    {
    case VC_CMD_START:
        StartEncoder();
        break;
    case VC_CMD_STOP:
        StopEncoder();
        break;
    default:
        break;
    }
    return (VC_SUCCESS);
}

VC_STATUS FLACDevice::Notify(VC_EVENT* evt)
{
    Buffer* buf = m_input->GetFilledBuffer();
    WriteData(buf->GetData(), buf->GetSamples());
    m_input->RecycleBuffer(buf);
    return (VC_SUCCESS);
}

InputPort* FLACDevice::Input(int portno)
{
    return (m_input);
}

OutputPort* FLACDevice::Output(int portno)
{
    return (m_output);
}

VC_STATUS FLACDevice::StartEncoder()
{
    VC_MSG("Enter");
    if (m_ready)
    {
        return (VC_SUCCESS);
    }

    FLAC__StreamEncoderInitStatus init_status;
    setParameters();
    init_status = FLAC__stream_encoder_init_stream(m_encoder, FLACDevice::write_callback, NULL, NULL, NULL,
        (void*) this);
    VC_CHECK(init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK, return (VC_FAILURE), "ERROR: initializing encoder: %s",
        FLAC__StreamEncoderInitStatusString[init_status]);
    Buffer* buf = m_output->GetBuffer();
    if (buf)
    {
        buf->SetTag(TAG_START);
        m_output->PushBuffer(buf);
    }

    m_ready = true;

    return (VC_SUCCESS);
}

VC_STATUS FLACDevice::StopEncoder()
{
    VC_MSG("Enter");
    if (!m_ready)
    {
        return (VC_SUCCESS);
    }

    FLAC__stream_encoder_finish(m_encoder);
    Buffer* buf = m_output->GetBuffer();
    if (buf)
    {
        buf->SetTag(TAG_END);
        m_output->PushBuffer(buf);
    }
    m_cdata.samples = 0;
    m_ready = false;

    return (VC_SUCCESS);
}
VC_STATUS FLACDevice::setParameters()
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
            FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL
            || !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", "voiceCommand")
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

int FLACDevice::WriteData(void* data, int samples)
{
    VC_MSG("Enter");
    FLAC__byte* buffer;
    buffer = (FLAC__byte*) data;
    size_t left = (size_t) samples;
    FLAC__int32 pcm[READSIZE * 2];

    while (left)
    {
        size_t need = (left > READSIZE ? (size_t) READSIZE : (size_t) left);
        {
            size_t i;
            for (i = 0; i < need * NO_OF_CHANNELS; i++)
            {
                pcm[i] = (FLAC__int32) (((FLAC__int16) (FLAC__int8) buffer[2 * i + 1] << 8)
                    | (FLAC__int16) buffer[2 * i]);
            }
            FLAC__stream_encoder_process_interleaved(m_encoder, pcm, need);
        }
        left -= need;
        buffer += need * 2 * NO_OF_CHANNELS;
    }

    return (m_cdata.samples);
}

void FLACDevice::progress_callback(const FLAC__StreamEncoder *m_encoder, FLAC__uint64 bytes_written,
    FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
    (void) m_encoder, (void) client_data;

    static_cast<ClientData*>(client_data)->samples = samples_written;
    fprintf(stderr, "wrote %llu bytes, %llu samples, %u/%u frames\n", bytes_written, samples_written, frames_written,
        total_frames_estimate);
}

FLAC__StreamEncoderWriteStatus FLACDevice::write_callback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[],
    size_t bytes, unsigned samples, unsigned current_frame, void *client_data)
{
    FLACDevice* self = static_cast<FLACDevice*>(client_data);
    Buffer* buf = self->Output(0)->GetBuffer();
    if (buf)
    {
        buf->WriteData((void*) buffer, bytes);
        self->Output(0)->PushBuffer(buf);
    }
    return (FLAC__STREAM_ENCODER_WRITE_STATUS_OK);

}
