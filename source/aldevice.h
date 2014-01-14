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
#include <sys/time.h>
#include <ctime>
#include "FLAC/metadata.h"
#include "FLAC/stream_encoder.h"
#include <fstream>
#include "worker.h"

#define SAMPLE_RATE 16000
#define NO_OF_CHANNELS 2

class ALDevice: public WorkerThread
{
public:
    ALDevice();
    virtual ~ALDevice();
    VC_STATUS Init();
    VC_STATUS CreateWAV();
    void* GetData();
    void CreateFLAC();
    void PlayAudio();
    int GetNoSamples();
    void StartCapture();
    void StopCapture();
    VC_STATUS GetCaptureDeviceList();

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
    ALuint buf;
    ALuint source;
    bool m_running;

};



#endif /* ALDEVICE_H_ */
