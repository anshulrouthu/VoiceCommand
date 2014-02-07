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

#define SAMPLE_RATE 16000
#define NO_OF_CHANNELS 2

/**
 * Capture Device Class that captures audio data continuously.
 * This class is responsible for capture of raw audio pcm data
 * This is a wrapper function around OpenAL library
 */
class CaptureDevice: public WorkerThread, public ADevice
{
public:
    CaptureDevice(const char* name);
    virtual ~CaptureDevice();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Notify();
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
    virtual const char* c_str()
    {
        return (m_name);
    }
    virtual void Task();
    VC_STATUS OpenPlaybackDevice();
    VC_STATUS OpenCaptureDevice();
    const char* GetCaptureDevice();
    const char* GetPlaybackDevice();
    ALCdevice* m_playbackdev;
    ALCdevice* m_capturedev;
    bool m_running;
    Timer* m_timer;
    int m_threshold;
    Mutex m_mutex;
    ConditionVariable m_cv;
    const char* m_name;
    InputPort* m_input;
    OutputPort* m_output;

};

#endif /* ALDEVICE_H_ */
