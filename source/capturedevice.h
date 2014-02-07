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
#include "audio_processor.h"
#include "buffer.h"

#define SAMPLE_RATE 16000
#define NO_OF_CHANNELS 2

/**
 * Capture Device Class that captures audio data continuously.
 * This class is responsible for capture of raw audio pcm data
 * This is a wrapper function around OpenAL library
 */
class CaptureDevice: public WorkerThread
{
public:
    CaptureDevice(int threashold);
    virtual ~CaptureDevice();
    void StartCapture();
    void StopCapture();
    VC_STATUS GetCaptureDeviceList(char** list);
    VC_STATUS ThresholdSetup();

private:
    virtual const char* c_str()
    {
        return ("CaptureDevice");
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
    AudioProcessor* m_audioprocess;
    int m_threshold;
    Mutex m_mutex;
    ConditionVariable m_cv;

};

#endif /* ALDEVICE_H_ */
