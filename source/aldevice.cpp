/*
 * aldevice.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */
#include "aldevice.h"

ALDevice::ALDevice(): m_running(false)
{

}

ALDevice::~ALDevice()
{
    // Wait for the source to stop playing
        playState = AL_PLAYING;
        while (playState == AL_PLAYING)
        {
            printf("  source %d is playing...\r", source);
            fflush(stdout);
            alGetSourcei(source, AL_SOURCE_STATE, &playState);
            usleep(100000);
        }
        printf("\nDone with playback.\n");
        fflush(stdout);

    // Shut down OpenAL
        alDeleteSources(1, &source);
        alDeleteBuffers(1, &buf);
        alcMakeContextCurrent (NULL);
        alcCloseDevice(mainDev);
        alcCaptureCloseDevice(captureDev);
}
void ALDevice::init()
{
    captureBuffer=(void*)malloc(1048576);
    mainDev = alcOpenDevice(NULL);
    if (mainDev == NULL)
    {
        printf("Unable to open playback device!\n");
        exit(1);
    }

    devices = alcGetString(mainDev, ALC_DEVICE_SPECIFIER);
    printf("   opened device '%s'\n", devices);

    mainContext = alcCreateContext(mainDev, NULL);
    if (mainContext == NULL)
    {
        printf("Unable to create playback context!\n");
        exit(1);
    }

    printf("   created playback context\n");

    alcMakeContextCurrent(mainContext);
    alcProcessContext(mainContext);

    printf("Opening capture device:\n");
    captureDev = alcCaptureOpenDevice(NULL, 16000, AL_FORMAT_STEREO16, 800);
    if (captureDev == NULL)
    {
        printf("   Unable to open device!\n");
        exit(1);
    }
    devices = alcGetString(captureDev, ALC_CAPTURE_DEVICE_SPECIFIER);
    printf("   opened device %s\n", devices);

}

void ALDevice::Task()
{
    while (m_state)
    {
        alcCaptureStart(captureDev);
        samplesCaptured = 0;
        captureBufPtr = (ALshort*) captureBuffer;
        int sum = 0, j=1;
        ALshort* tmp;
        while (m_running)
        {
            alcGetIntegerv(captureDev, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);
            if (samplesAvailable > 0)
            {
                alcCaptureSamples(captureDev, captureBufPtr, samplesAvailable);
                samplesCaptured += samplesAvailable;
                captureBufPtr += samplesAvailable * 2;
            }

            if (j / 100)
            {
                fflush(stdout);
                if (sum)
                    printf("amplitude:%d\n", sum / j);
                tmp = captureBufPtr;
                sum = 0;
                j = 0;
            }
            if(tmp)
            {
                sum += abs((int) *tmp++);
                j++;
            }
            //if(sum/j>1000)
            //break;
            // Wait for a bit
            //usleep(10000);

            // Update the clock
            currentTime = time(NULL);
        }
    }
}

void ALDevice::startCapture()
{
    start();
    usleep(1000000);
    m_running = true;
}

void ALDevice::stopCapture()
{
    m_running=false;
    join();
    alcCaptureStop(captureDev);
}

void ALDevice::createWAV()
{
    printf("Creating WaV file...");
    alGenBuffers(1, &buf);
    alGenSources(1, &source);
    alBufferData(buf, AL_FORMAT_STEREO16, (ALshort*) captureBuffer, samplesCaptured * 2, 16000);
    alSourcei(source, AL_BUFFER, buf);
    alSourcePlay(source);

    // Wait for the source to stop playing
        playState = AL_PLAYING;
        while (playState == AL_PLAYING)
        {
            printf("  source %d is playing...\r", source);
            //fflush(stdout);
            alGetSourcei(source, AL_SOURCE_STATE, &playState);
            usleep(100000);
        }
}

int ALDevice::getNoSamples()
{
    return (samplesCaptured);
}

void* ALDevice::getData()
{
    return captureBuffer;
}
