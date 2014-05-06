/*********************************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file curldevice.cpp is part of VoiceCommand project

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
 * curldevice.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: anshul
 */

#include "curldevice.h"

CURLDevice::CURLDevice(std::string name, const char* filename) :
    ADevice(name),
    m_header(NULL),
    m_formpost(NULL),
    m_filename(filename),
    m_buffer(NULL)
{
    curl_global_init(CURL_GLOBAL_ALL);
    m_curl = curl_easy_init();

    VC_CHECK(m_curl == NULL,, "Error Initializing Curl");
    m_header = curl_slist_append(m_header, "Content-type: audio/x-flac; rate=44100");
    m_header = curl_slist_append(m_header, "Transfer-Encoding: chunked");

}

CURLDevice::~CURLDevice()
{
    /* this mutex is required by the main thread waiting on a condition
     take the mutex wake up main thread and release the mutex */
    {
        AutoMutex automutex(&m_mutex);
        m_cv.Notify();
    }

    Join();

    curl_easy_cleanup(m_curl);
    curl_slist_free_all(m_header);
    curl_formfree(m_formpost);

    delete m_input;
    delete m_output;
}

VC_STATUS CURLDevice::Initialize()
{
    VC_TRACE("Enter");

    m_input = new InputPort("Curl Input", this);
    m_output = new OutputPort("Curl Output", this);

    curl_easy_setopt(m_curl, CURLOPT_URL, VC_SPEECH_ENGINEV2);
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_header);
    curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, CURLDevice::read_callback);
    curl_easy_setopt(m_curl, CURLOPT_READDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, CURLDevice::Write_callback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 0L);

    return (VC_SUCCESS);
}

VC_STATUS CURLDevice::Notify(VC_EVENT* evt)
{
    AutoMutex automutex(&m_mutex);
    m_cv.Notify();

    return (VC_SUCCESS);
}

InputPort* CURLDevice::Input(int portno)
{
    return (m_input);
}

OutputPort* CURLDevice::Output(int portno)
{
    return (m_output);
}

VC_STATUS CURLDevice::SendCommand(VC_CMD cmd)
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

void CURLDevice::Task()
{
    while (m_state)
    {
        if (m_input->IsBufferAvailable())
        {
            Buffer* buf = m_input->GetFilledBuffer();
            if (buf->GetTag() == TAG_START)
            {
                m_buffer = Output(0)->GetBuffer();
                VC_CHECK(curl_easy_perform(m_curl) != CURLE_OK,, "Error requesting command");

                char* str = strstr((char*) m_buffer->GetData(), "\n") + 1;

                if (str)
                {
                    m_buffer->Reset();
                    m_buffer->WriteData((void*) str, strlen(str));
                    m_buffer->WriteData((char*)"\0",1);
                }

                VC_MSG("Received Json Text: %s", m_buffer->GetData());
                Output(0)->PushBuffer(m_buffer);
                m_buffer = NULL;
            }
            m_input->RecycleBuffer(buf);
        }
        else
        {
            AutoMutex automutex(&m_mutex);
            while (!m_input->IsBufferAvailable() && m_state)
            {
                m_cv.Wait();
            }
        }
    }
}

size_t CURLDevice::Write_callback(void* buffer, size_t size, size_t n, void* ptr)
{
    VC_TRACE_STATIC("Enter");

    CURLDevice* self = static_cast<CURLDevice*>(ptr);
    self->m_buffer->WriteData(buffer, size * n);

    return (size * n);
}

size_t CURLDevice::read_callback(void *buffer, size_t size, size_t n, void *ptr)
{
    VC_CHECK_STATIC(size * n < 1, return (0), "Error");

    size = 0;
    CURLDevice* self = static_cast<CURLDevice*>(ptr);

    if (!self->Input(0)->IsBufferAvailable())
    {
        AutoMutex automutex(&self->m_mutex);
        while (!self->Input(0)->IsBufferAvailable() && self->m_state)
        {
            self->m_cv.Wait();
        }
    }

    Buffer* buf = self->Input(0)->GetFilledBuffer();

    if (buf)
    {
        if (buf->GetTag() == TAG_END || buf->GetTag() == TAG_BREAK)
        {
            DBGPRINT(DBG_MESSAGE, ("%s Received %s\n",__FUNCTION__, ConvertTagToString(buf->GetTag())));
            return (0);
        }

        size = buf->GetSize();
        memcpy(buffer, buf->GetData(), size);
        self->Input(0)->RecycleBuffer(buf);
    }

    return (size);

}

