/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
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

class InputPort;
class OutputPort;
class ADevice;

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
    VC_STATUS ConnectDevices(ADevice* src, ADevice* dst);
    VC_STATUS DisconnectDevices(ADevice* src, ADevice* dst);
    VC_STATUS ConnectPorts(InputPort* input, OutputPort* output);
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
    ;
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

class ADevice
{
public:
    ADevice()
    {
    }
    ;
    virtual ~ADevice()
    {
    }
    virtual VC_STATUS Initialize()
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Function to notify the device for any events
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

};

/**
 * An InputPort to a device that receives data.
 * Also takes care of the buffer management
 */
class InputPort
{
public:
    InputPort(std::string name, ADevice* device);
    ~InputPort()
    {
    }
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

};

/**
 * OutputPort to a device that sends the data out
 */
class OutputPort
{
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

#endif /* APIPE_H_ */
