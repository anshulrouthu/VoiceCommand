/***********************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file curldevice.h is part of VoiceCommand project

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
#define VC_SPEECH_ENGINEV2 "https://www.google.com/speech-api/v2/recognize?output=json&key=AIzaSyCnl6MRydhw_5fLXIdASxkLJzcJh5iX0M4&client=chromium&pfilter=0&maxresults=1&lang=\"en-us\""

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

private:
    virtual void Task();
    static size_t Write_callback(void *buffer, size_t size, size_t n, void *ptr);
    static size_t read_callback(void *buffer, size_t size, size_t n, void *ptr);
    CURL* m_curl;
    struct curl_slist *m_header;
    struct curl_httppost* m_formpost;
    const char* m_filename;
    InputPort* m_input;
    OutputPort* m_output;
    Buffer* m_buffer;

};

#endif /* CURL_H_ */
