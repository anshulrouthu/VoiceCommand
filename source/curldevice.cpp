/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * curldevice.cpp
 *
 *  Created on: Feb 16, 2014
 *      Author: anshul
 */

#include "curldevice.h"

CURLDevice::CURLDevice(std::string name, const char* filename) :
    m_header(NULL),
    m_formpost(NULL),
    m_name(name),
    m_filename(filename),
    m_cv(m_mutex)
{
    curl_global_init(CURL_GLOBAL_ALL);
    m_curl = curl_easy_init();

    VC_CHECK(m_curl == NULL,, "Error Initializing Curl");
    m_header = curl_slist_append(m_header, "Content-type: audio/x-flac; rate=16000");

    m_buffer = (char*) malloc(400);
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
    if (m_buffer)
        free(m_buffer);

    delete m_input;
    delete m_output;
}

VC_STATUS CURLDevice::Initialize()
{
    VC_TRACE("Enter");

    m_input = new InputPort("Curl Input", this);
    m_output = new OutputPort("Curl Output", this);

    //m_header = curl_slist_append(m_header, "Content-type: audio/x-flac; rate=16000");
    m_header = curl_slist_append(m_header, "Transfer-Encoding: chunked");

    curl_easy_setopt(m_curl, CURLOPT_URL, VC_SPEECH_ENGINE);
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_header);
    curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, CURLDevice::read_callback);
    curl_easy_setopt(m_curl, CURLOPT_READDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, CURLDevice::Write_callback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

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
                VC_CHECK(curl_easy_perform(m_curl) != CURLE_OK,, "Error requesting command");
            }
        }
        else
        {
            AutoMutex automutex(&m_mutex);
            m_cv.Wait();
        }
    }
}

char* CURLDevice::GetText()
{
    VC_MSG("Enter");
    struct curl_httppost *lastptr = NULL;
    curl_formadd(&m_formpost, &lastptr, CURLFORM_COPYNAME, "sendfile", CURLFORM_FILE, "audio.flac", CURLFORM_END);

    curl_easy_setopt(m_curl, CURLOPT_URL, VC_SPEECH_ENGINE);
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_header);
    curl_easy_setopt(m_curl, CURLOPT_HTTPPOST, m_formpost);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, CURLDevice::WriteData);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, (void* )m_buffer);
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 0);
    VC_CHECK(curl_easy_perform(m_curl) != CURLE_OK, return (NULL), "Error requesting command");

    return (m_buffer);

}

size_t CURLDevice::WriteData(void* buffer, size_t size, size_t n, void* ptr)
{
    DBG_PRINT(DBG_TRACE, "Enter %d", size * n);
    //memcpy(ptr,buffer,size*n);
    strcpy((char*) ptr, (char*) buffer);
    DBG_PRINT(DBG_TRACE, "Command Data %s", (char* )buffer);
    return (size * n);
}

size_t CURLDevice::Write_callback(void* buffer, size_t size, size_t n, void* ptr)
{
    VC_TRACE_STATIC("Enter");

    CURLDevice* self = static_cast<CURLDevice*>(ptr);
    Buffer* buf = self->Output(0)->GetBuffer();
    buf->WriteData(buffer, size * n);
    self->Output(0)->PushBuffer(buf);

    VC_ALL_STATIC("Received Json Text %s", (char* )buffer);

    return (size * n);
}

size_t CURLDevice::read_callback(void *buffer, size_t size, size_t n, void *ptr)
{
    VC_CHECK_STATIC(size * n < 1, return (0), "Error");

    CURLDevice* self = static_cast<CURLDevice*>(ptr);

    if (!self->Input(0)->IsBufferAvailable())
    {
        while (!self->Input(0)->IsBufferAvailable())
        {
            AutoMutex automutex(&self->m_mutex);
            self->m_cv.Wait();
        }
    }

    Buffer* buf = self->Input(0)->GetFilledBuffer();

    if (buf->GetTag() == TAG_END || buf->GetTag() == TAG_BREAK)
    {
        DBGPRINT(DBG_ALWAYS, ("Received TAG_EOS\n"));
        return (0);
    }

    size = buf->GetSize();
    memcpy(buffer, buf->GetData(), size);
    self->Input(0)->RecycleBuffer(buf);

    return (size);

}

