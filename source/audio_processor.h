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

class AudioProcessor: public WorkerThread, public ADevice
{
public:
    AudioProcessor(std::string name);
    virtual ~AudioProcessor();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual InputPort* Input(int portno);
    virtual OutputPort* Output(int portno);
    virtual VC_STATUS SendCommand(VC_CMD cmd);
    virtual VC_STATUS SetParameters(const InputParams* params);
    virtual VC_STATUS GetParameters(OutputParams* params);

    VC_STATUS InitiateDataProcessing();
    VC_STATUS CloseDataProcessing(char* text);

    const char* c_str()
    {
        return (m_name.c_str());
    }
private:

    virtual void Task();
    FLACDevice* m_flac;
    CURLDevice* m_curl;
    Json::Reader m_reader;
    char m_text[4 * 1024];
    Mutex m_mutex;
    ConditionVariable m_cv;
    std::string m_name;
    InputPort* m_input;
    OutputPort* m_output;
};

#endif /*AUDIO_PROCESSOR_H_*/
