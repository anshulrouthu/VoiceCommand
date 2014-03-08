/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * curl.h
 *
 *  Created on: Feb 16, 2014
 *      Author: anshul
 */

#ifndef CURL_H_
#define CURL_H_

#include "utils.h"
#include "curl/curl.h"
#include "apipe.h"
#include "worker.h"

#define VC_SPEECH_ENGINE "https://www.google.com/speech-api/v1/recognize?xjerr=1&client=chromium&pfilter=0&maxresults=1&lang=\"en-US\""
#define VC_AUDIO_FILENAME "audio.flac"

class CURLDevice: public ADevice, public WorkerThread
{
public:
    CURLDevice(std::string name, const char* filename = "audio.flac");
    virtual ~CURLDevice();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual InputPort* Input(int portno);
    virtual OutputPort* Output(int portno);
    virtual VC_STATUS SendCommand(VC_CMD cmd);
    char* GetText();

private:
    virtual void Task();
    static size_t Write_callback(void *buffer, size_t size, size_t n, void *ptr);
    static size_t read_callback(void *buffer, size_t size, size_t n, void *ptr);
    static size_t WriteData(void *buffer, size_t size, size_t n, void *ptr);
    CURL* m_curl;
    struct curl_slist *m_header;
    char* m_buffer;
    struct curl_httppost* m_formpost;
    const char* m_filename;
    InputPort* m_input;
    OutputPort* m_output;

};

#endif /* CURL_H_ */