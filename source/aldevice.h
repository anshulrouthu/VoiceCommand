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

class ALDevice: public WorkerThread
{
public:
    ALDevice();
    virtual ~ALDevice();
    void init();
    void startCapture();
    void stopCapture();
    void* getData();
    void createWAV();
    void CreateFLAC();
    void playAudio();
    int getNoSamples();

private:
    virtual void Task();
    const ALCchar* devices;
    ALCdevice* mainDev;
    ALCcontext* mainContext;
    ALCdevice* captureDev;
    void* captureBuffer;
    ALshort*captureBufPtr;
    ALint samplesAvailable;
    ALint samplesCaptured;
    time_t currentTime;
    time_t lastTime;
    ALuint buf;
    ALuint source;
    ALint playState;
    int i;
    bool m_running;

};



#endif /* ALDEVICE_H_ */
