/*
 * aldevice.h
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */

#ifndef ALDEVICE_H_
#define ALDEVICE_H_

#include "utils.h"
#include <AL/al.h>
#include <AL/alc.h>
#include "flac.h"
#include <fstream>
#include "worker.h"
#include "audio_processor.h"
#include "buffer.h"

#define SAMPLE_RATE 16000
#define NO_OF_CHANNELS 2

class ALDevice: public WorkerThread
{
public:
    ALDevice(int threashold);
    virtual ~ALDevice();
    void* GetData();
    void CreateFLAC();
    void PlayAudio();
    int GetNoSamples();
    void StartCapture();
    void StopCapture();
    VC_STATUS GetCaptureDeviceList();
    VC_STATUS ThresholdSetup();

private:
    virtual const char*  c_str()
    {
        return ("ALDevice");
    }
    VC_STATUS OpenPlaybackDevice();
    VC_STATUS OpenCaptureDevice();
    const char* GetCaptureDevice();
    const char* GetPlaybackDevice();
    virtual void Task();
    ALCdevice* m_playbackdev;
    ALCdevice* m_capturedev;
    void* m_captureBuffer;
    ALint m_samplescaptured;
    bool m_running;
    Timer* m_timer;
    AudioProcessor* m_audioprocess;
    int m_threshold;
    char* m_text;

};



#endif /* ALDEVICE_H_ */
