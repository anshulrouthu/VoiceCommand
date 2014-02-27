/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
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
    std::string JSONToText(Buffer* buf);
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
