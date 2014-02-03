/*
 * aldevice.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */
#include "aldevice.h"

ALDevice::ALDevice(int threshold) :
    m_running(false),m_threshold(threshold)
{
    m_captureBuffer = (void*) malloc(5*1024*1024);

    VC_CHECK(!m_captureBuffer,,"Error allocating m_captureBuffer");
    m_audioprocess = new AudioProcessor();
    m_timer = new Timer();
    m_text = (char*)malloc(200*sizeof(char));
    VC_CHECK(!m_text,,"Error allocating m_text");
    OpenCaptureDevice();
}

ALDevice::~ALDevice()
{
    alcMakeContextCurrent(NULL);
    alcCloseDevice(m_playbackdev);
    alcCaptureCloseDevice(m_capturedev);
    //if(m_captureBuffer)
      //  free(m_captureBuffer);
    if(m_text)
        free(m_text);

    delete m_audioprocess;
    delete m_timer;

}

VC_STATUS ALDevice::ThresholdSetup()
{
    VC_MSG("Initiating Calibration...");
    int i = 0,k=0;
    ALshort* ptr;
    ALint samplesAvailable;
    m_samplescaptured = 0;
    ptr = (ALshort*) m_captureBuffer;
    int sum = 0, th[] = {0,0};
    ALshort* tmp = ptr;
    alcCaptureStart(m_capturedev);

iteration2:

    time_t lastTime,currentTime;
    for (int j = 4; j > 0; j--)
    {
        if(i==0)
        {
            printf("Please say 'Hello VoiceCommand' in %d sec... \r",j);
        }
        else
        {
            printf("Please be quite in %d sec... \r",j);
            i=0;
        }

        fflush(stdout);
        lastTime = time(NULL);
        currentTime = lastTime;
        while (currentTime == lastTime)
        {
            currentTime = time(NULL);
            usleep(100000);
        }
    }

    m_timer->ResetTimer();
    m_timer->StartTimer();
    sum = 0;
    while (m_timer->GetTimePassed() < 1.5)
    {
        alcGetIntegerv(m_capturedev, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);
        if (samplesAvailable > 0)
        {
            i++;
            tmp = ptr;
            sum = 0;
            alcCaptureSamples(m_capturedev, ptr, samplesAvailable);
            m_samplescaptured += samplesAvailable;
            ptr += samplesAvailable * 2;

            while (tmp <= ptr)
            {
                sum += abs((int) *tmp++);
            }

            th[k] +=sum/samplesAvailable;
            VC_TRACE("amplitude:%d", sum / samplesAvailable);
        }
    }
    th[k]/=i;
    //m_audioprocess->ProcessAudioData(GetData(), GetNoSamples());
    if(strcmp(m_text,"hello voice command"))
    {
        VC_MSG("Auto Setup is unsuccessful");
        //return (VC_STATUS);
    }
    VC_MSG("Auto Setup is completed. Threshold level is %d %d %d",th[0],th[1],(th[0]+th[1])/2);
    if(!k)
    {
        k++;
        goto iteration2;
    }

    m_threshold = (th[0]+th[1])/2;
    return (VC_SUCCESS);
}

VC_STATUS ALDevice::OpenPlaybackDevice()
{
    VC_MSG("Enter");
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
    VC_MSG("Enter");

    m_capturedev = alcCaptureOpenDevice(NULL, SAMPLE_RATE, AL_FORMAT_STEREO16, 800);

    VC_CHECK(m_capturedev == NULL,return (VC_FAILURE),"Unable to open capture device!");
    VC_TRACE("opened device %s", GetCaptureDevice());

    return (VC_SUCCESS);
}

VC_STATUS ALDevice::GetCaptureDeviceList()
{
    printf("Available capture devices:\n");
    const char *ptr = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

    while (ptr[0] != 0)
        while (*ptr)
        {
            printf("   %s\n", ptr);
            ptr += strlen(ptr) + 1;
        }
    return (VC_SUCCESS);
}

void ALDevice::Task()
{
    VC_MSG("Enter");
    Buffer* buf =  m_audioprocess->GetBuffer();

    while (m_state)
    {
        ALint samplesAvailable;
        bool process_data = false;
        m_samplescaptured = 0;
        ALshort* ptr = (ALshort*) buf->GetData();
        int total_samples=0;

        alcCaptureStart(m_capturedev);
        m_timer->StartTimer();

        while (m_running)
        {
            alcGetIntegerv(m_capturedev, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);

            if (samplesAvailable > 0)
            {
                alcCaptureSamples(m_capturedev, ptr, samplesAvailable);
                m_samplescaptured += samplesAvailable;
                total_samples+=samplesAvailable;

                ALshort* tmp = (ALshort*)ptr;
                int sum = 0, avg_amplitude = 0;

                for(ALint i = 0; i <= samplesAvailable * 2; i++)
                {
                    sum += abs((int) *tmp++);
                }

                avg_amplitude = sum / samplesAvailable;

                if (avg_amplitude > m_threshold) //checking the amplitude/volume greater that threashold
                {
                    m_timer->ResetTimer();
                    VC_MSG("Timer Reset");
                    if(!process_data)
                    {
                        Buffer* b = m_audioprocess->GetBuffer();
                        b->SetTag(TAG_START);
                        m_audioprocess->PushBuffer(b);
                        process_data = true;
                    }

                    VC_TRACE("amplitude:%d %f", avg_amplitude, m_timer->GetTimePassed());
                }

                ptr += samplesAvailable* 2;

                if(process_data && m_samplescaptured > 2048)
                {
                    buf->SetSamples(m_samplescaptured);
                    m_audioprocess->PushBuffer(buf);
                    m_samplescaptured = 0;
                    buf = m_audioprocess->GetBuffer();
                    ptr = (ALshort*)buf->GetData();
                }
                else if(!process_data)
                {
                    usleep(50000);
                }

                if(total_samples > 40000 && (avg_amplitude < m_threshold * 3 / 8))
                {
                    Buffer* b = m_audioprocess->GetBuffer();
                    b->SetTag(TAG_BREAK);
                    m_audioprocess->PushBuffer(b);
                    total_samples = 0;
                }

                if (m_timer->GetTimePassed() >= 800)
                {
                    VC_MSG("TimeOut");
                    if(process_data)
                    {
                        alcCaptureStop(m_capturedev);
                        Buffer* b = m_audioprocess->GetBuffer();
                        b->SetTag(TAG_END);
                        m_audioprocess->PushBuffer(b);
                    }
                    break;
                }
            }
            else
            {
                usleep(50000);
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
