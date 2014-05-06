/*********************************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file apipe.cpp is part of VoiceCommand project

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

#include "apipe.h"
#include "capturedevice.h"
#include "audio_processor.h"
#include "file_io.h"
#include "flac.h"

/**
 * Number of input buffers
 */
#define NUM_OF_BUFFERS 128

/**
 * APipe constructor
 * @param name to identify the pipe
 */
APipe::APipe(std::string name) :
    m_name(name)
{
}

/**
 * Returns the queried device based on the VC_DEVICETYPE
 * @param[in] devtype type of device requested
 * @param[in] name of the device to be names for identification
 * @return device instance of the device available based on type
 */
ADevice* APipe::GetDevice(VC_DEVICETYPE devtype, std::string name, const char* filename)
{
    VC_TRACE("Enter");
    switch (devtype)
    {
    case VC_CAPTURE_DEVICE:
        return (new CaptureDevice(name));
        break;
    case VC_AUDIO_PROCESSOR:
        return (new AudioProcessor(name, this));
        break;
    case VC_FLAC_DEVICE:
        return (new FLACDevice(name));
        break;
    case VC_CURL_DEVICE:
        return (new CURLDevice(name));
        break;
    case VC_TEXT_PROCESSOR:
        break;
    case VC_COMMAND_PROCESSOR:
        break;
    case VC_FILESINK_DEVICE:
        return (new FileSink(name, filename));
        break;
    case VC_FILESRC_DEVICE:
        return (new FileSrc(name, filename));
        break;
    default:
        break;
    }

    return (NULL);
}

/**
 * Connects the devices to their default ports i.e port 0
 * @param src source device
 * @param dst destination device
 */
VC_STATUS APipe::ConnectDevices(ADevice* src, ADevice* dst, int src_portno, int dst_portno)
{
    VC_TRACE("Enter");
    VC_CHECK(!src || !dst, return (VC_FAILURE), "Error: Null parameters");
    return (ConnectPorts(dst->Input(dst_portno), src->Output(src_portno)));
}

/**
 * Disconnects the devices from their default ports i.e port 0
 * @param src source device
 * @param dst destination device
 */
VC_STATUS APipe::DisconnectDevices(ADevice* src, ADevice* dst, int src_portno, int dst_portno)
{
    VC_TRACE("Enter");
    VC_CHECK(!src || !dst, return (VC_FAILURE), "Error: Null parameters");
    return (DisconnectPorts(dst->Input(dst_portno), src->Output(src_portno)));
}

/**
 * Connect the specific ports irrespective of device's default
 * @param input port
 * @param output port
 */
VC_STATUS APipe::ConnectPorts(InputPort* input, OutputPort* output)
{
    VC_TRACE("Enter");
    VC_CHECK(!input || !output, return (VC_FAILURE), "Error: Null parameters");
    return (output->SetReceiver(input));
}

/**
 * Disconnect the specific ports irrespective of device's default
 * @param input port
 * @param output port
 */
VC_STATUS APipe::DisconnectPorts(InputPort* input, OutputPort* output)
{
    VC_TRACE("Enter");
    VC_CHECK(!input || !output, return (VC_FAILURE), "Error: Null parameters");
    VC_CHECK(output->m_receiver != input, return (VC_FAILURE), "Error: Invalid ports");
    return (output->SetReceiver(NULL));
}

/**
 * Inputport constructor
 * @param name to identify this input port
 * @param device this input port belongs to
 */
InputPort::InputPort(std::string name, ADevice* device) :
    m_name(name),
    m_device(device),
    m_queue_cv(m_queue_mutex)
{
    for (int i = 0; i < NUM_OF_BUFFERS; i++)
    {
        Buffer* buf = new Buffer();
        m_buffers.push_back(buf);
    }
}

InputPort::~InputPort()
{
    AutoMutex automutex(&m_queue_mutex);

    for (std::list<Buffer*>::iterator it = m_buffers.begin(); it != m_buffers.end(); it++)
    {
        delete *it;
    }

    for (std::list<Buffer*>::iterator it = m_processbuf.begin(); it != m_processbuf.end(); it++)
    {
        delete *it;
    }

    m_buffers.clear();
}
/**
 * Returns the filled buffer from the received buffer queue
 * @return buf to be processed by device
 */
Buffer* InputPort::GetFilledBuffer()
{
    VC_TRACE("Enter");

    AutoMutex automutex(&m_queue_mutex);
    VC_CHECK(m_processbuf.size() == 0, return (NULL), "No buffers available to be processed");

    Buffer* buf = m_processbuf.front();
    m_processbuf.pop_front();

    return (buf);
}

/**
 * Returns an empty buffer. from the pre-allocated buffer pool to be filled with data
 * @return buf, data to be filled in
 */
Buffer* InputPort::GetEmptyBuffer()
{
    AutoMutex automutex(&m_queue_mutex);
    while (m_buffers.size() == 0)
    {
        //this should not happen or we starve for buffer
        VC_ERR("Waiting for buffers");
        m_queue_cv.Wait();
    }

    Buffer* buf = m_buffers.front();
    m_buffers.pop_front();

    return (buf);
}

/**
 * Recycles the buffer and stores in the empty buffer pool
 * @param buf to be recycled
 */
VC_STATUS InputPort::RecycleBuffer(Buffer* buf)
{
    VC_TRACE("Enter");
    AutoMutex automutex(&m_queue_mutex);
    buf->Reset();
    m_buffers.push_back(buf);
    m_queue_cv.Notify();
    return (VC_SUCCESS);
}

/**
 * Receives buffer from the connected port
 * @param buf received buffer
 */
VC_STATUS InputPort::ReceiveBuffer(Buffer* buf)
{
    VC_TRACE("Enter");
    {
        AutoMutex automutex(&m_queue_mutex);
        m_processbuf.push_back(buf);
    }

    /*
     * This mutex should be released before Notify call, some device
     * block this call, and access the input buffer e.g filesink device
     * TODO: Improve this notification mechanism
     */
    if (m_device)
    {
        m_device->Notify(NULL);
    }

    return (VC_SUCCESS);
}

/**
 * Checks if a buffer is available for processing
 * @return true/false
 */
bool InputPort::IsBufferAvailable()
{
    VC_TRACE("Enter");
    AutoMutex automutex(&m_queue_mutex);
    return (m_processbuf.size());
}

/**
 * Outputport constructor
 * @param name to identify this output port
 * @param device this output port belongs to
 */
OutputPort::OutputPort(std::string name, ADevice* device) :
    m_name(name),
    m_device(device),
    m_receiver(NULL)
{
}

/**
 * Sets the receiver of the output port
 * @param inport the input port to connect to
 */
VC_STATUS OutputPort::SetReceiver(InputPort* inport)
{
    VC_TRACE("Enter");
    if ((m_receiver && !inport) || (!m_receiver && inport))
    {
        m_receiver = inport;
        return (VC_SUCCESS);
    }
    VC_ERR("Error: Cannot connect receiver");
    return (VC_FAILURE);
}

/**
 * Pushes the buffer to is receiver (connected input port)
 * @param buf a buffer to be pushed
 */
VC_STATUS OutputPort::PushBuffer(Buffer* buf)
{
    VC_TRACE("Enter");
    if (m_receiver)
    {
        return (m_receiver->ReceiveBuffer(buf));
    }
    return (VC_FAILURE);
}

/**
 * Gets the buffer from the connected input port for data to be filled
 * @return buf
 */
Buffer* OutputPort::GetBuffer()
{
    VC_TRACE("Enter");
    if (m_receiver)
    {
        return (m_receiver->GetEmptyBuffer());
    }

    return (NULL);
}

/**
 * return the buffer to input port if the buffer is not used
 * @param buf
 */
VC_STATUS OutputPort::ReturnBuffer(Buffer* buf)
{
    return (m_receiver->RecycleBuffer(buf));
}
