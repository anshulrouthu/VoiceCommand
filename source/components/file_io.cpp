/*********************************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file file_io.cpp is part of VoiceCommand project

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
 * file_capture.cpp
 *
 *  Created on: Feb 8, 2014
 *      Author: anshul
 */
#include "file_io.h"

FileSink::FileSink(std::string name, const char* filename) :
    ADevice(name),
    m_filename(filename)
{
}

FileSink::~FileSink()
{
    fclose(m_file);
    delete m_input;
}

/**
 * Initialize the file capture device
 */
VC_STATUS FileSink::Initialize()
{
    VC_TRACE("Enter");
    m_input = new InputPort("FileSink Input", this);
    m_file = fopen(m_filename, "wb");
    return (VC_SUCCESS);
}

/**
 * Write data in to a file
 */
VC_STATUS FileSink::WriteData()
{
    VC_TRACE("Enter");
    while (m_input->IsBufferAvailable())
    {
        Buffer* buf = m_input->GetFilledBuffer();
        if (buf->GetTag() == TAG_NONE)
        {
            fwrite(buf->GetData(), buf->GetSize(), 1, m_file);
        }
        m_input->RecycleBuffer(buf);
    }
    return (VC_SUCCESS);
}

/**
 * Get the input port
 */
InputPort* FileSink::Input(int portno)
{
    VC_TRACE("Enter");
    return (m_input);
}

/**
 * Get the output port
 */
OutputPort* FileSink::Output(int portno)
{
    VC_TRACE("Enter");
    return (m_output);
}

/**
 * Notify the device about an event
 */
VC_STATUS FileSink::Notify(VC_EVENT* evt)
{
    VC_TRACE("Enter");
    WriteData();
    return (VC_SUCCESS);
}

/**
 * Send command to the device
 */
VC_STATUS FileSink::SendCommand(VC_CMD cmd)
{
    VC_TRACE("Enter");
    return (VC_SUCCESS);
}

FileSrc::FileSrc(std::string name, const char* in_file) :
    ADevice(name),
    m_filename(in_file)
{
}

FileSrc::~FileSrc()
{
    fclose(m_file);
    delete m_output;
}

VC_STATUS FileSrc::Initialize()
{
    VC_TRACE("Enter");
    m_output = new OutputPort("FileSrc Output", this);
    m_file = fopen(m_filename, "rb");
    VC_CHECK(!m_file, return (VC_FAILURE), "Error opening file %s", m_filename);
    return (VC_SUCCESS);
}

OutputPort* FileSrc::Output(int portno)
{
    return (m_output);
}

VC_STATUS FileSrc::SendCommand(VC_CMD cmd)
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

void FileSrc::Task()
{
    VC_TRACE("Enter");

    Buffer* buf = m_output->GetBuffer();
    buf->SetTag(TAG_START);
    m_output->PushBuffer(buf);

    while (m_state)
    {
        Buffer* buf = m_output->GetBuffer();
        size_t size = fread(buf->GetData(), 1, 1024, m_file);

        VC_TRACE("Size of data read %d", size);

        if (size > 0)
        {
            buf->SetSize(size);
            VC_CHECK(m_output->PushBuffer(buf) != VC_SUCCESS,, "Failed to push buffer");
        }
        else
        {
            VC_TRACE("End of file reached");
            buf->SetTag(TAG_END);
            m_output->PushBuffer(buf);
            break;
        }
    }

}
