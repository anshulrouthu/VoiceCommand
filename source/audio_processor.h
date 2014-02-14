/***********************************************************
voiceCommand 

  Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

  All rights reserved.

  This software is distributed on an "AS IS" BASIS, 
  WITHOUT  WARRANTIES OR CONDITIONS OF ANY KIND, either 
  express or implied.
***********************************************************/

#ifndef AUDIO_PROCESSOR_H_
#define AUDIO_PROCESSOR_H_

#include "utils.h"
#include "flac.h"
#include "timer.h"
#include <json/json.h>
#include "curl/curl.h"
#include "buffer.h"
#include "worker.h"
#include "apipe.h"

#define VC_SPEECH_ENGINE "https://www.google.com/speech-api/v1/recognize?xjerr=1&client=chromium&pfilter=0&maxresults=1&lang=\"en-US\""
#define VC_AUDIO_FILENAME "audio.flac"

class CURLWrapper;

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
    FLACWrapper* m_flac;
    CURLWrapper* m_curl;
    Json::Reader m_reader;
    char m_text[4*1024];
    Mutex m_mutex;
    ConditionVariable m_cv;
    std::string m_name;
    InputPort* m_input;
    OutputPort* m_output;
};


class CURLWrapper
{
public:
    CURLWrapper(char* filename);
    ~CURLWrapper();
    char* GetText();
    static size_t WriteData(void *buffer, size_t size, size_t n, void *ptr);

private:
    const char* c_str()
    {
        return ("CURLWrapper");
    }
    CURL* m_curl;
    struct curl_slist *m_header;
    char* m_buffer;
    struct curl_httppost* m_formpost;
};

#endif /*AUDIO_PROCESSOR_H_*/
