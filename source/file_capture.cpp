/***********************************************************
voiceCommand 

  Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

  All rights reserved.

  This software is distributed on an "AS IS" BASIS, 
  WITHOUT  WARRANTIES OR CONDITIONS OF ANY KIND, either 
  express or implied.
***********************************************************/

/*
 * file_capture.cpp
 *
 *  Created on: Feb 8, 2014
 *      Author: anshul
 */
#include "file_capture.h"

FileCapture::FileCapture(std::string name) :
    m_name(name)
{
}

FileCapture::~FileCapture()
{
    fclose(m_file);
    delete m_input;
}

/**
 * Initialize the file capture device
 */
VC_STATUS FileCapture::Initialize()
{
    VC_TRACE("Enter");
    m_input = new InputPort("File Input 0", this);
    m_file = fopen((m_name + ".out").c_str(), "wb");
    return (VC_SUCCESS);
}

/**
 * Write data in to a file
 */
VC_STATUS FileCapture::WriteData()
{
    VC_TRACE("Enter");
    while (m_input->IsBufferAvailable())
    {
        Buffer* buf = m_input->GetFilledBuffer();
        if (buf->GetTag() == TAG_NONE)
        {
            fwrite(buf->GetData(), sizeof(char), buf->GetSize(), m_file);
        }
        m_input->RecycleBuffer(buf);
    }
    return (VC_SUCCESS);
}

/**
 * Get the input port
 */
InputPort* FileCapture::Input(int portno)
{
    VC_TRACE("Enter");
    return (m_input);
}

/**
 * Get the output port
 */
OutputPort* FileCapture::Output(int portno)
{
    VC_TRACE("Enter");
    return (m_output);
}

/**
 * Notify the device about an event
 */
VC_STATUS FileCapture::Notify(VC_EVENT* evt)
{
    VC_TRACE("Enter");
    WriteData();
    return (VC_SUCCESS);
}

/**
 * Send command to the device
 */
VC_STATUS FileCapture::SendCommand(VC_CMD cmd)
{
    VC_TRACE("Enter");
    return (VC_SUCCESS);
}
