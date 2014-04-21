/***********************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file apipe.h is part of VoiceCommand project

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
 * apipe.h
 *
 *  Created on: Feb 5, 2014
 *      Author: anshul
 */

#ifndef APIPE_H_
#define APIPE_H_

#include "utils.h"
#include "buffer.h"
#include "mutex.h"

class ADevice;
class APipe;

/**
 * An InputPort to a device that receives data.
 * Also takes care of the buffer management
 */
class InputPort
{

public:
    InputPort(std::string name, ADevice* device);
    ~InputPort();

    Buffer* GetFilledBuffer();
    Buffer* GetEmptyBuffer();
    bool IsBufferAvailable();
    VC_STATUS RecycleBuffer(Buffer* buf);
    VC_STATUS ReceiveBuffer(Buffer* buf);
    const char* c_str()
    {
        return (m_name.c_str());
    }
private:
    std::list<Buffer*> m_buffers;
    std::list<Buffer*> m_processbuf;
    std::string m_name;
    ADevice* m_device;
    Mutex m_queue_mutex;

};

/**
 * OutputPort to a device that sends the data out
 */
class OutputPort
{
    friend class APipe;
public:
    OutputPort(std::string name, ADevice* device);
    ~OutputPort()
    {
    }
    VC_STATUS PushBuffer(Buffer* buf);
    Buffer* GetBuffer();
    VC_STATUS SetReceiver(InputPort* inport);
    VC_STATUS ReturnBuffer(Buffer* buf);
    const char* c_str()
    {
        return (m_name.c_str());
    }
private:
    std::string m_name;
    ADevice* m_device;
    InputPort* m_receiver;
};

/**
 * Input parameters for the devices
 */
typedef struct
{
    int threshold; //Threshold audio input level
} InputParams;

/**
 * Output parameters from the devices
 */
typedef struct
{
    char** device_list; //Hardware devices on the system
} OutputParams;

/**
 * A pipe that maintains all the devices and their connections
 */
class APipe
{
public:
    APipe(std::string name);
    ~APipe()
    {
    }

    /**
     * Query the pipe for available devices
     */
    ADevice* GetDevice(VC_DEVICETYPE dev, std::string name, const char* filename = "");

    /**
     * Connects the devices to their default ports i.e port 0
     * @param src source device
     * @param dst destination device
     */
    VC_STATUS ConnectDevices(ADevice* src, ADevice* dst, int src_portno = 0, int dst_portno = 0);

    /**
     * Disconnects the devices from their default ports i.e port 0
     * @param src source device
     * @param dst destination device
     */
    VC_STATUS DisconnectDevices(ADevice* src, ADevice* dst, int src_portno = 0, int dst_portno = 0);

    /**
     * Connect the specific ports irrespective of device's default
     * @param input port
     * @param output port
     */
    VC_STATUS ConnectPorts(InputPort* input, OutputPort* output);

    /**
     * Disconnect the specific ports irrespective of device's default
     * @param input port
     * @param output port
     */
    VC_STATUS DisconnectPorts(InputPort* input, OutputPort* output);

    const char* c_str()
    {
        return (m_name.c_str());
    }
private:
    std::string m_name;
};

/**
 * An interface to all the devices
 */
class BaseDevice
{
public:
    BaseDevice()
    {
    }

    virtual ~BaseDevice()
    {
    }

    virtual VC_STATUS Initialize()=0;

    /**
     * Function to notify the device for any events
     */
    virtual VC_STATUS Notify(VC_EVENT* evt)=0;

    /**
     * Get the inputport of the device
     */
    virtual InputPort* Input(int portno)=0;

    /**
     * Get the output port of the device
     */
    virtual OutputPort* Output(int portno)=0;

    /**
     * Send a command to device. This method triggers the device to start or stop
     */
    virtual VC_STATUS SendCommand(VC_CMD cmd)=0;

    /**
     * Sets the required parameters for device
     */
    virtual VC_STATUS SetParameters(const InputParams* params)=0;

    /**
     * Gets the required parameters from device
     */
    virtual VC_STATUS GetParameters(OutputParams* params)=0;

};

/**
 * A Base class for all the Devices in the application
 */
class ADevice: public BaseDevice
{
public:
    /**
     * ADevice Constructor
     */
    ADevice(std::string name, APipe* pipe = NULL) :
        m_name(name),
        m_cv(m_mutex),
        m_pipe(pipe)
    {
    }

    /**
     * ADevice Destructor
     */
    virtual ~ADevice()
    {
    }

    /**
     * Initialize the device, calls the childs implementation
     */
    virtual VC_STATUS Initialize()
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Function to notify the device for any events
     * TODO: update the api to notify different type of events, as needed
     */
    virtual VC_STATUS Notify(VC_EVENT* evt)
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Get the inputport of the device
     */
    virtual InputPort* Input(int portno)
    {
        return (NULL);
    }

    /**
     * Get the output port of the device
     */
    virtual OutputPort* Output(int portno)
    {
        return (NULL);
    }

    /**
     * Send a command to device. This method triggers the device to start or stop
     */
    virtual VC_STATUS SendCommand(VC_CMD cmd)
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Sets the required parameters for device
     */
    virtual VC_STATUS SetParameters(const InputParams* params)
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Gets the required parameters from device
     */
    virtual VC_STATUS GetParameters(OutputParams* params)
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Name of the device for debugging purpose
     */
    virtual const char* c_str()
    {
        return (m_name.c_str());
    }

protected:
    std::string m_name;
    Mutex m_mutex;
    ConditionVariable m_cv;
    APipe* m_pipe;

};

#endif /* APIPE_H_ */
