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
#include "flac.h"
#include "timer.h"
#include <json/json.h>
#include "buffer.h"
#include "worker.h"
#include "apipe.h"
#include "curldevice.h"
#include "mutex.h"

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

    bool IsBufferAvailable();
    std::string JSONToText(Buffer* buf);
    VC_STATUS CloseDataProcessing(char* text);

private:

    virtual void Task();
    Json::Reader m_reader;
    char m_text[4 * 1024];
    FLACDevice* m_flac;
    CURLDevice* m_curl;
    OutputPort* m_output;
    std::map<int,InputPort*> m_input_map;
};

#endif /*AUDIO_PROCESSOR_H_*/
