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

    m_captureBuffer = (void*) malloc(5*1024*1024);
    m_audioprocess = new AudioProcessor();
    m_timer = new Timer();
    //OpenPlaybackDevice();
    OpenCaptureDevice();

}

ALDevice::~ALDevice()
{
    alcMakeContextCurrent(NULL);
    alcCloseDevice(m_playbackdev);
    alcCaptureCloseDevice(m_capturedev);
    free(m_captureBuffer);
    delete m_audioprocess;
    delete m_timer;
}

VC_STATUS ALDevice::Init()
{
    return (VC_SUCCESS);
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
    while (m_state)
    {
        ALshort* captureBufPtr;
        ALint samplesAvailable;
        int amplitude = 0;
        bool process_data = false;
        m_samplescaptured = 0;
        captureBufPtr = (ALshort*) m_captureBuffer;
        int sum = 0, j = 1;
        ALshort* tmp = captureBufPtr;
        m_timer->ResetTimer();
        m_timer->StartTimer();
        alcCaptureStart(m_capturedev);
        while (m_running)
        {
            alcGetIntegerv(m_capturedev, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);

            if (samplesAvailable > 0)
            {
                tmp = captureBufPtr;
                sum = 0;
                alcCaptureSamples(m_capturedev, captureBufPtr, samplesAvailable);
                m_samplescaptured += samplesAvailable;
                captureBufPtr += samplesAvailable * 2;

                while (tmp <= captureBufPtr)
                {
                    sum += abs((int) *tmp++);
                }

                if (sum / samplesAvailable > 1200) //checking the amplitude/volume greater that threashold
                {
                    m_timer->ResetTimer();
                    process_data = true;
                    VC_TRACE("amplitude:%d %f", sum / samplesAvailable, m_timer->GetTimePassed());
                    VC_TRACE("Timer Reset");
                }

                if (m_timer->GetTimePassed() >= 0.5)
                {
                    VC_TRACE("TimeOut");
                    break;
                }
            }
        }

        if(process_data)
        {
            VC_TRACE("Processing data");
            const char *cmd = m_audioprocess->ProcessAudioData(GetData(),GetNoSamples());
            if(!strcmp(cmd,"exit") || !strcmp(cmd,"cu") || !strcmp(cmd,"see you later") || !strcmp(cmd,"bye bye"))
            {
                VC_ALL("Exit command");
                StopCapture();
            }
        }
    }
}

void ALDevice::StartCapture()
{
    start();
    usleep(100000);
    m_running = true;
}

void ALDevice::StopCapture()
{
    m_running = false;
    stop();
    alcCaptureStop(m_capturedev);
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
