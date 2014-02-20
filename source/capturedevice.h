/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/**
 * @file capturedevice.h
 *
 * Capturedevice is the main source for the audio data to the application.
 * This device captured the audio data when ever there is any
 *
 *   Created on: Jan 9, 2014
 *       Author: anshul routhu <anshul.m67@gmail.com>
 */

#ifndef ALDEVICE_H_
#define ALDEVICE_H_

#include "utils.h"
#include <AL/al.h>
#include <AL/alc.h>
#include "flac.h"
#include "worker.h"
#include "buffer.h"
#include "apipe.h"
#include "timer.h"
#include "mutex.h"

#define SAMPLE_RATE 16000

/**
 * Capture Device Class that captures audio data continuously.
 * This class is responsible for capture of raw audio pcm data
 * This is a wrapper function around OpenAL library
 */
class CaptureDevice: public WorkerThread, public ADevice
{
public:
    CaptureDevice(std::string name);
    virtual ~CaptureDevice();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual InputPort* Input(int portno);
    virtual OutputPort* Output(int portno);
    virtual VC_STATUS SendCommand(VC_CMD cmd);
    virtual VC_STATUS SetParameters(const InputParams* params);
    virtual VC_STATUS GetParameters(OutputParams* params);

    void StartCapture();
    void StopCapture();
    VC_STATUS GetCaptureDeviceList(char** list);
    VC_STATUS ThresholdSetup();

private:
    virtual void Task();
    VC_STATUS OpenPlaybackDevice();
    VC_STATUS OpenCaptureDevice();
    const char* GetCaptureDevice();
    const char* GetPlaybackDevice();
    ALCdevice* m_playbackdev;
    ALCdevice* m_capturedev;
    bool m_running;
    int m_threshold;
    Timer* m_timer;
    InputPort* m_input;
    OutputPort* m_output;

};

#endif /* ALDEVICE_H_ */
