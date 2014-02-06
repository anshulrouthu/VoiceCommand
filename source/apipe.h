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
#include "capturedevice.h"
#include "audio_processor.h"

class ADevice;
class OutputPort;
class Inputport;

/**
 * An InputPort to a device that receives data
 */
class InputPort
{
public:
    InputPort(char* name, ADevice* device);
    ~InputPort()
    {
    }
    Buffer* GetFilledBuffer();
    Buffer* GetEmptyBuffer();
    bool IsBufferAvailable();
    VC_STATUS RecycleBuffer(Buffer* buf);
    VC_STATUS ReceiveBuffer(Buffer* buf);
private:
    const char* c_str()
    {
        return (m_name);
    }
    std::list<Buffer*> m_buffers;
    std::list<Buffer*> m_processbuf;
    char* m_name;
    ADevice* m_device;

};

/**
 * OutputPort to a device that sends the data out
 */
class OutputPort
{
public:
    OutputPort(char* name, ADevice* device);
    ~OutputPort()
    {
    }
    VC_STATUS PushBuffer(Buffer* buf);
    Buffer* GetBuffer();
    VC_STATUS SetReceiver(InputPort* inport);
private:
    const char* c_str()
    {
        return (m_name);
    }
    char* m_name;
    ADevice* m_device;
    InputPort* m_receiver;
};

/**
 * A pipe that maintains all the devices and their connections
 */
class APipe
{
public:
    APipe(char* name);
    ~APipe(){}
    /**
     * Query the pipe for devices
     */
    ADevice* GetDevice(VC_DEVICETYPE dev);

    VC_STATUS ConnectDevices(ADevice* src, ADevice* dst);
    VC_STATUS DisconnectDevices(ADevice* src, ADevice* dst);
    VC_STATUS ConnectPorts(InputPort* input,OutputPort* output);
    VC_STATUS DisconnectPorts(InputPort* input,OutputPort* output);
private:
    char* m_name;
};

/**
 * An interface to all the devices
 */
class ADevice
{
public:
    ADevice(char* name);
    virtual ~ADevice();
    virtual VC_STATUS Initialize()=0;
    /**
     * Function to notify the device for any events
     */
    virtual VC_STATUS Notify()=0;

    /**
     * Get the inputport of the device
     */
    virtual InputPort* Input(int portno)=0;

    /**
     * Get the output port of the device
     */
    virtual OutputPort* Output(int portno)=0;

};

#endif /* APIPE_H_ */
