/*
 * apipe.cpp
 *
 *  Created on: Feb 5, 2014
 *      Author: anshul
 */

#include "apipe.h"
/**
 * Number of input buffers
 */
#define NUM_OF_BUFFERS 128

/**
 * Inputport constructor
 * @param name to identify this input port
 * @param device this input port belongs to
 */
InputPort::InputPort(char* name, ADevice* device) :
    m_name(name),
    m_device(device)
{
    for (int i = 0; i < NUM_OF_BUFFERS; i++)
    {
        Buffer* buf = new Buffer();
        m_buffers.push_back(buf);
    }
}

/**
 * Returns the filled buffer from the received buffer queue
 * @return buf to be processed by device
 */
Buffer* InputPort::GetFilledBuffer()
{
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
    while (m_buffers.size() == 0)
    {
        VC_ERR("Low on Buffers");
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
    buf->Reset();
    m_buffers.push_back(buf);
    return (VC_SUCCESS);
}

/**
 * Receives buffer from the connected port
 * @param buf received buffer
 */
VC_STATUS InputPort::ReceiveBuffer(Buffer* buf)
{
    VC_TRACE("Enter");
    m_processbuf.push_back(buf);

    m_device->Notify();

    return (VC_SUCCESS);
}

/**
 * Checks if a buffer is abailable for processing
 * @return true/false
 */
bool InputPort::IsBufferAvailable()
{
    return (m_processbuf.size());
}

/**
 * Outputport constructor
 * @param name to identify this output port
 * @param device this output port belongs to
 */
OutputPort::OutputPort(char* name, ADevice* device) :
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
    m_receiver = inport;
    return (VC_SUCCESS);
}

/**
 * Pushes the buffer to is receiver (connected input port)
 * @param buf a buffer to be pushed
 */
VC_STATUS OutputPort::PushBuffer(Buffer* buf)
{
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
    if (m_receiver)
    {
        return (m_receiver->GetEmptyBuffer());
    }

    VC_ERR("Not connected to any InputPort");
    return (NULL);
}

/**
 * APipe constructor
 * @param name to identify the pipe
 */
APipe::APipe(char* name):
    m_name(name)
{
}

/**
 * Returns the queried device based on the VC_DEVICETYPE
 * @param[in] devtype type of device requested
 * @return device instance of the device available based on type
 */
ADevice* APipe::GetDevice(VC_DEVICETYPE devtype)
{
    switch(devtype)
    {
    case VC_CAPTURE_DEVICE:
        break;
    case VC_AUDIO_PROCESSOR:
        break;
    case VC_TEXT_PROCESSOR:
        break;
    case VC_COMMAND_PROCESSOR:
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
VC_STATUS APipe::ConnectDevices(ADevice* src, ADevice* dst)
{
    return (ConnectPorts(dst->Input(0),src->Output(0)));
}

/**
 * Disconnects the devices from their default ports i.e port 0
 * @param src source device
 * @param dst destination device
 */
VC_STATUS APipe::DisconnectDevices(ADevice* src, ADevice* dst)
{
    return (DisconnectPorts(dst->Input(0),src->Output(0)));
}

/**
 * Connect the specific ports irrespective of devices
 * @param input port
 * @param output port
 */
VC_STATUS APipe::ConnectPorts(InputPort* input, OutputPort* output)
{
    return (output->SetReceiver(input));
}

/**
 * Disconnect the specific ports irrespective of devices
 * @param input port
 * @param output port
 */
VC_STATUS APipe::DisconnectPorts(InputPort* input, OutputPort* output)
{
    return (output->SetReceiver(NULL));
}
