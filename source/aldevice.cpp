/*
 * aldevice.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */
#include "aldevice.h"

ALDevice::ALDevice() :
    m_running(false)
{
}

ALDevice::~ALDevice()
{
    // Wait for the source to stop playing
    ALint playState;
    playState = AL_PLAYING;
    while (playState == AL_PLAYING)
    {
        //VC_TRACE("source %d is playing...", source);
        alGetSourcei(source, AL_SOURCE_STATE, &playState);
        usleep(100000);
    }
    VC_TRACE("Done with playback.");

    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buf);
    alcMakeContextCurrent(NULL);
    alcCloseDevice(m_playbackdev);
    alcCaptureCloseDevice(m_capturedev);
}

VC_STATUS ALDevice::Init()
{
    VC_STATUS status = VC_SUCCESS;
    m_captureBuffer = (void*) malloc(1048576);

    GetCaptureDeviceList();
    status = OpenPlaybackDevice();
    status = OpenCaptureDevice();

    return (status);
}

VC_STATUS ALDevice::OpenPlaybackDevice()
{
    ALCcontext* mainctx;
    m_playbackdev = alcOpenDevice(NULL);

    VC_CHECK(m_playbackdev == NULL, return (VC_FAILURE),"Unable to open playback device!");
    VC_TRACE("opened device '%s'",GetPlaybackDevice());

    mainctx = alcCreateContext(m_playbackdev, NULL);

    VC_CHECK(mainctx == NULL, return (VC_FAILURE),"Unable to create playback context!");
    VC_TRACE("created playback context");

    alcMakeContextCurrent(mainctx);
    alcProcessContext(mainctx);

    return (VC_SUCCESS);
}

VC_STATUS ALDevice::OpenCaptureDevice()
{
    VC_TRACE("Opening capture device:");

    m_capturedev = alcCaptureOpenDevice(NULL, SAMPLE_RATE, AL_FORMAT_STEREO16, 800);

    VC_CHECK(m_capturedev == NULL,return (VC_FAILURE),"Unable to open capture device!");
    VC_TRACE("opened device %s", GetCaptureDevice());

    return (VC_SUCCESS);
}

VC_STATUS ALDevice::GetCaptureDeviceList()
{
    printf("Available capture devices:\n");
    const char *ptr = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

    while (ptr[0] != NULL)
        while (*ptr)
        {
            printf("   %s\n", ptr);
            ptr += strlen(ptr) + 1;
        }
    return (VC_SUCCESS);
}

void ALDevice::Task()
{
    ALshort* captureBufPtr;
    ALint samplesAvailable;

    while (m_state)
    {
        alcCaptureStart(m_capturedev);
        m_samplescaptured = 0;
        captureBufPtr = (ALshort*) m_captureBuffer;
        int sum = 0, j = 1;
        ALshort* tmp;
        while (m_running)
        {
            alcGetIntegerv(m_capturedev, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);
            if (samplesAvailable > 0)
            {
                alcCaptureSamples(m_capturedev, captureBufPtr, samplesAvailable);
                m_samplescaptured += samplesAvailable;
                captureBufPtr += samplesAvailable * 2;
            }

            if (j / 100)
            {
                if (sum)
                    VC_ALL("amplitude:%d", sum / j);
                tmp = captureBufPtr;
                sum = 0;
                j = 0;
            }
            if (tmp)
            {
                sum += abs((int) *tmp++);
                j++;
            }
            //if(sum/j>1000)
            //break;
            // Wait for a bit
            //usleep(10000);

            // Update the clock
        }
    }
}

void ALDevice::StartCapture()
{
    start();
    usleep(1000000);
    m_running = true;
}

void ALDevice::StopCapture()
{
    m_running = false;
    join();
    alcCaptureStop(m_capturedev);
}

VC_STATUS ALDevice::CreateWAV()
{
    ALint playState;
    VC_ALL("Creating WaV file...");
    alGenBuffers(1, &buf);
    alGenSources(1, &source);
    alBufferData(buf, AL_FORMAT_STEREO16, (ALshort*) m_captureBuffer, m_samplescaptured * 2, SAMPLE_RATE);
    alSourcei(source, AL_BUFFER, buf);
    alSourcePlay(source);

    // Wait for the source to stop playing
    playState = AL_PLAYING;
    while (playState == AL_PLAYING)
    {
        VC_ALL("source %d is playing...\r", source);
        alGetSourcei(source, AL_SOURCE_STATE, &playState);
        usleep(100000);
    }

    return (VC_SUCCESS);
}

int ALDevice::GetNoSamples()
{
    return (m_samplescaptured);
}

void* ALDevice::GetData()
{
    return (m_captureBuffer);
}

const char* ALDevice::GetCaptureDevice()
{
    return (alcGetString(m_capturedev, ALC_CAPTURE_DEVICE_SPECIFIER));
}

const char* ALDevice::GetPlaybackDevice()
{
    return (alcGetString(m_playbackdev, ALC_CAPTURE_DEVICE_SPECIFIER));
}
