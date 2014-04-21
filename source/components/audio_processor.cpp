/*********************************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file audio_processor.cpp is part of VoiceCommand project

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

#include "audio_processor.h"

AudioProcessor::AudioProcessor(std::string name, APipe* pipe) :
    ADevice(name, pipe),
    m_curl(NULL),
    m_encoder(NULL),
    m_ready(false)
{
}

AudioProcessor::~AudioProcessor()
{
    /* this mutex is required by the main thread waiting on a condition
     take the mutex wake up main thread and release the mutex */
    {
        AutoMutex automutex(&m_mutex);
        m_cv.Notify();
    }

    Join();

    m_curl->SendCommand(VC_CMD_STOP);

    m_pipe->DisconnectDevices(this, m_curl, 1, 0);
    m_pipe->DisconnectDevices(m_curl, this, 0, 1);

    FLAC__metadata_object_delete(m_metadata[0]);
    FLAC__metadata_object_delete(m_metadata[1]);
    FLAC__stream_encoder_delete(m_encoder);

    delete m_curl;

    //delete output ports
    for (std::map<int, OutputPort*>::iterator it = m_output_map.begin(); it != m_output_map.end(); it++)
    {
        delete (*it).second;
        (*it).second = NULL;
    }

    //delete input ports
    for (std::map<int, InputPort*>::iterator it = m_input_map.begin(); it != m_input_map.end(); it++)
    {
        delete (*it).second;
        (*it).second = NULL;
    }

}

/**
 * Initialize the device and get all the resources
 */
VC_STATUS AudioProcessor::Initialize()
{
    m_curl = new CURLDevice("CurlDevice");

    m_encoder = FLAC__stream_encoder_new();
    VC_CHECK(m_encoder == NULL,, "Error: Unable to initialize FLAC encoder");
    SetupEncoder();

    for (int i = 0; i < 2; i++)
    {
        InputPort* m_input = new InputPort("AudProc Input_" + ItoString(i), this);
        m_input_map[i] = m_input;
    }

    for (int i = 0; i < 2; i++)
    {
        OutputPort* m_output = new OutputPort("AudProc Output_" + ItoString(i), this);
        m_output_map[i] = m_output;
    }

    m_curl->Initialize();

    m_pipe->ConnectDevices(this, m_curl, 1, 0);
    m_pipe->ConnectDevices(m_curl, this, 0, 1);

    m_curl->SendCommand(VC_CMD_START);

    return (VC_SUCCESS);
}

/**
 * Return the input port of the device
 * @return m_input
 */
InputPort* AudioProcessor::Input(int portno)
{
    return (m_input_map[portno]);
}

/**
 * Return the output port of the device
 * @return m_output
 */
OutputPort* AudioProcessor::Output(int portno)
{
    return (m_output_map[portno]);
}

/**
 * Notifies the device of any event
 */
VC_STATUS AudioProcessor::Notify(VC_EVENT* evt)
{
    AutoMutex automutex(&m_mutex);
    m_cv.Notify();

    return (VC_SUCCESS);
}

/**
 * Send a command to the device
 */
VC_STATUS AudioProcessor::SendCommand(VC_CMD cmd)
{
    switch (cmd)
    {
    case VC_CMD_START:
        Start();
        break;
    case VC_CMD_STOP:
        Stop();
        break;
    }
    return (VC_SUCCESS);
}

/**
 * Set the required parameters for device
 * @paran params
 */
VC_STATUS AudioProcessor::SetParameters(const InputParams* params)
{
    return (VC_SUCCESS);
}

/**
 * Get the required parameters from device
 * @paran params
 */
VC_STATUS AudioProcessor::GetParameters(OutputParams* params)
{
    return (VC_SUCCESS);
}

/**
 * Method to convert json text to string
 */
std::string AudioProcessor::JSONToText(Buffer* buf)
{
    Json::Value root;
    Json::Value hypotheses;
    std::string utterance;
    double confidence;
    char *cmd;

    cmd = (char*) buf->GetData();
    if (cmd)
    {
        VC_CHECK(!m_reader.parse(cmd, root, true), return (""), "Error parsing text");

        hypotheses = root["hypotheses"][(unsigned int) (0)];

        if (hypotheses["confidence"].isDouble())
        {
            confidence = hypotheses["confidence"].asDouble();
            VC_MSG("Confidence %f", confidence);
        }

        if (confidence > 0.7 && hypotheses["utterance"].isString())
        {
            utterance = hypotheses["utterance"].asString();
            cmd[0] = '\0';
            VC_MSG("\n\tUtterance: %s\n\tConfidence: %f", utterance.c_str(), confidence);
            return (utterance);
        }
    }

    return ("");
}

void AudioProcessor::Task()
{
    VC_MSG("Enter");
    bool text_break = false;
    std::string text = "";
    while (m_state)
    {
        for (std::map<int, InputPort*>::iterator it = m_input_map.begin(); it != m_input_map.end(); it++)
        {
            InputPort* input = (*it).second;
            if (input->IsBufferAvailable())
            {
                if ((*it).first == 0)
                {
                    //first input port connected to capture device
                    Buffer* buf = input->GetFilledBuffer();

                    switch (buf->GetTag())
                    {
                    case TAG_START:
                        text = "\0";
                        StartEncoder();
                        break;
                    case TAG_BREAK:
                        text_break = true;
                        StopEncoder();
                        StartEncoder();
                        break;
                    case TAG_END:
                        text_break = false;
                        StopEncoder();
                        break;
                    case TAG_NONE:
                        WriteData(buf->GetData(), buf->GetSamples());
                        break;
                    default:
                        break;
                    }

                    input->RecycleBuffer(buf);
                }
                else
                {
                    //second input port connected to curldevice
                    Buffer* buf = input->GetFilledBuffer();

                    if (text_break)
                    {
                        text += " " + JSONToText(buf);
                    }
                    else
                    {
                        text = JSONToText(buf);
                    }

                    VC_ALL("Got Text: %s", text.c_str());
                    input->RecycleBuffer(buf);
                }
            }
            else
            {
                //wait condition
                AutoMutex automutex(&m_mutex);
                while (!IsBufferAvailable() && m_state)
                {
                    m_cv.Wait();
                }
            }
        }
    }
}

/**
 * Method to check is there is any buffer in any input port to be processed
 */
bool AudioProcessor::IsBufferAvailable()
{
    for (std::map<int, InputPort*>::iterator it = m_input_map.begin(); it != m_input_map.end(); it++)
    {
        if ((*it).second->IsBufferAvailable())
        {
            return (true);
        }
    }
    return (false);
}

/**
 * Encode the raw pcm data
 * @param[in] data raw pcm input data
 * @param[in] samples number of samples
 */
int AudioProcessor::WriteData(void* data, int samples)
{
    VC_MSG("Enter");
    FLAC__byte* buffer;
    buffer = (FLAC__byte*) data;
    size_t left = (size_t) samples;
    FLAC__int32 pcm[READSIZE * NO_OF_CHANNELS];

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

    return (0);
}

/**
 * Set parameters for the flac encoder
 */
VC_STATUS AudioProcessor::SetupEncoder()
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

/**
 * Stop encoding data
 */
VC_STATUS AudioProcessor::StopEncoder()
{
    VC_MSG("Enter");
    if (!m_ready)
    {
        return (VC_SUCCESS);
    }

    FLAC__stream_encoder_finish(m_encoder);
    Buffer* buf = Output(1)->GetBuffer();
    if (buf)
    {
        buf->SetTag(TAG_END);
        Output(1)->PushBuffer(buf);
    }
    m_ready = false;

    return (VC_SUCCESS);
}

/**
 * Start encoding data
 */
VC_STATUS AudioProcessor::StartEncoder()
{
    VC_MSG("Enter");
    if (m_ready)
    {
        return (VC_SUCCESS);
    }

    FLAC__StreamEncoderInitStatus init_status;
    SetupEncoder();
    init_status = FLAC__stream_encoder_init_stream(m_encoder, this->write_callback, NULL, NULL, NULL, (void*) this);
    VC_CHECK(init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK, return (VC_FAILURE), "ERROR: initializing encoder: %s",
        FLAC__StreamEncoderInitStatusString[init_status]);

    Buffer* buf = Output(1)->GetBuffer();
    if (buf)
    {
        buf->SetTag(TAG_START);
        Output(1)->PushBuffer(buf);
    }

    m_ready = true;

    return (VC_SUCCESS);
}

/**
 * Callback from flac encoder with the encoded data. This encoded data is to be sent to curl device
 */
FLAC__StreamEncoderWriteStatus AudioProcessor::write_callback(const FLAC__StreamEncoder *encoder,
    const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data)
{
    ADevice* self = static_cast<AudioProcessor*>(client_data);
    Buffer* buf = self->Output(1)->GetBuffer();
    if (buf)
    {
        buf->WriteData((void*) buffer, bytes);
        self->Output(1)->PushBuffer(buf);
    }
    return (FLAC__STREAM_ENCODER_WRITE_STATUS_OK);

}
