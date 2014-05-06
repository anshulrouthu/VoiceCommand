/***********************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file audio_processor.h is part of VoiceCommand project

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

#ifndef AUDIO_PROCESSOR_H_
#define AUDIO_PROCESSOR_H_

#include "utils.h"
#include "timer.h"
#include <json/json.h>
#include "buffer.h"
#include "worker.h"
#include "apipe.h"
#include "curldevice.h"
#include "mutex.h"
#include "FLAC/metadata.h"
#include "FLAC/stream_encoder.h"

#define READSIZE 512
#define SAMPLE_RATE 16000
#define BITS_PER_SECOND 16

class AudioProcessor: public WorkerThread, public ADevice
{
public:
    AudioProcessor(std::string name, APipe* pipe = NULL);
    virtual ~AudioProcessor();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual InputPort* Input(int portno);
    virtual OutputPort* Output(int portno);
    virtual VC_STATUS SendCommand(VC_CMD cmd);
    virtual VC_STATUS SetParameters(const InputParams* params);
    virtual VC_STATUS GetParameters(OutputParams* params);

private:
    static FLAC__StreamEncoderWriteStatus write_callback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[],
        size_t bytes, unsigned samples, unsigned current_frame, void *client_data);
    virtual void Task();
    int WriteData(void* data, int total_samples);
    VC_STATUS SetupEncoder();
    VC_STATUS StopEncoder();
    VC_STATUS StartEncoder();
    bool IsBufferAvailable();
    VC_STATUS JSONToText(Buffer* buf, std::string& text);
    VC_STATUS CloseDataProcessing(char* text);
    Json::Reader m_reader;
    char m_text[4 * 1024];
    ADevice* m_curl;
    std::map<int, InputPort*> m_input_map;
    std::map<int, OutputPort*> m_output_map;
    FLAC__StreamEncoder* m_encoder;
    FLAC__StreamMetadata* m_metadata[2];
    bool m_ready;
};

#endif /*AUDIO_PROCESSOR_H_*/
