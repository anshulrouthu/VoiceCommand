/***********************************************************
voiceCommand 

  Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

  All rights reserved.

  This software is distributed on an "AS IS" BASIS, 
  WITHOUT  WARRANTIES OR CONDITIONS OF ANY KIND, either 
  express or implied.
***********************************************************/

/**
 * @file capturedevice.cpp
 *
 * Capturedevice is the main source for the audio data to the application.
 * This device captured the audio data when ever there is any
 *
 *   Created on: Jan 9, 2014
 *       Author: anshul routhu <anshul.m67@gmail.com>
 */

#include "capturedevice.h"

/**
 * Capturedevice constructor. Initializes the threashold level of input audio level
 * @param thr threshold input level
 */
CaptureDevice::CaptureDevice(std::string name) :
    m_running(false),
    m_threshold(2000),
    m_cv(m_mutex),
    m_name(name)
{
}

/**
 * Capturedevice Destructor
 */
CaptureDevice::~CaptureDevice()
{
    alcMakeContextCurrent(NULL);
    alcCloseDevice(m_playbackdev);
    alcCaptureCloseDevice(m_capturedev);

    delete m_timer;
    delete m_input;
    delete m_output;

}

/**
 * Initialize the device and get all the resources
 */
VC_STATUS CaptureDevice::Initialize()
{
    m_timer = new Timer();
    m_input = new InputPort("Inputport 0", this);
    m_output = new OutputPort("Ouputport 0", this);

    OpenCaptureDevice();

    return (VC_SUCCESS);
}

/**
 * Return the input port of the device
 * @return m_input
 */
InputPort* CaptureDevice::Input(int portno)
{
    return (m_input);
}

/**
 * Return the output port of the device
 * @return m_output
 */
OutputPort* CaptureDevice::Output(int portno)
{
    return (m_output);
}

/**
 * Notifies the device of any event
 */
VC_STATUS CaptureDevice::Notify(VC_EVENT* evt)
{
    //TODO: update the api to notify different type of events
    m_mutex.Lock();
    m_cv.Notify();
    m_mutex.Unlock();

    return (VC_SUCCESS);
}

/**
 * Send a command to the device
 */
VC_STATUS CaptureDevice::SendCommand(VC_CMD cmd)
{
    switch (cmd)
    {
    case VC_CMD_START:
        Start();
        usleep(10000);
        m_running = true;
        break;
    case VC_CMD_STOP:
        m_running = false;
        Stop();
        alcCaptureStop(m_capturedev);
        break;
    }
    return (VC_SUCCESS);
}

/**
 * Set the required parameters for device
 * @paran params
 */
VC_STATUS CaptureDevice::SetParameters(const InputParams* params)
{
    VC_TRACE("Enter");
    m_threshold = params->threshold;
    return (VC_SUCCESS);
}

/**
 * Get the required parameters from device
 * @paran params
 */
VC_STATUS CaptureDevice::GetParameters(OutputParams* params)
{
    return (VC_SUCCESS);
}

/**
 * Function that sets up the threshold audio input level
 * @return VC_SUCCESS on successful execution
 */
VC_STATUS CaptureDevice::ThresholdSetup()
{
    return (VC_NOT_IMPLEMENTED);
}

/**
 * Function opens a PlaybackDevice in the system
 * @return VC_SUCCESS on successful execution
 */
VC_STATUS CaptureDevice::OpenPlaybackDevice()
{
    VC_MSG("Enter");
    ALCcontext* mainctx;
    m_playbackdev = alcOpenDevice(NULL);

    VC_CHECK(m_playbackdev == NULL, return (VC_FAILURE), "Unable to open playback device!");
    VC_TRACE("opened device '%s'", GetPlaybackDevice());

    mainctx = alcCreateContext(m_playbackdev, NULL);

    VC_CHECK(mainctx == NULL, return (VC_FAILURE), "Unable to create playback context!");
    VC_TRACE("created playback context");

    alcMakeContextCurrent(mainctx);
    alcProcessContext(mainctx);

    return (VC_SUCCESS);
}

/**
 * Function opens CaptureDevice in the system
 * @return VC_SUCCESS on successful execution
 */
VC_STATUS CaptureDevice::OpenCaptureDevice()
{
    VC_MSG("Enter");

    m_capturedev = alcCaptureOpenDevice(NULL, SAMPLE_RATE, AL_FORMAT_MONO16, 800);

    VC_CHECK(m_capturedev == NULL, return (VC_FAILURE), "Unable to open capture device!");
    VC_TRACE("opened device %s", GetCaptureDevice());

    return (VC_SUCCESS);
}

/**
 * Function gets the list of capturedevices in the system
 * @param[out] list of all available capture devices
 */
VC_STATUS CaptureDevice::GetCaptureDeviceList(char** list)
{
    printf("Available capture devices:\n");
    const char *ptr = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

    while (ptr[0] != 0)
    {
        for (int i = 0; *ptr; i++)
        {
            strcpy(list[i], ptr);
            VC_TRACE("   %s\n", ptr);
            ptr += strlen(ptr) + 1;
        }
    }
    return (VC_SUCCESS);
}

/**
 * Thread that captures data.
 * This threads looks for any valid data with amplitude more than
 * threshold value, and fulls the buffer with data and sends to audioprocessor module
 */
void CaptureDevice::Task()
{
    VC_MSG("Enter");
    Buffer* buf = m_output->GetBuffer();

    while (m_state)
    {
        ALint samplesAvailable;
        bool process_data = false;
        int samples_2k = 0;
        ALshort* ptr = (ALshort*) buf->GetData();
        int total_samples = 0;

        alcCaptureStart(m_capturedev);
        /* start the timer */
        m_timer->StartTimer();

        while (m_running)
        {
            alcGetIntegerv(m_capturedev, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);

            /* if there are any samples */
            if (samplesAvailable > 0)
            {
                alcCaptureSamples(m_capturedev, ptr, samplesAvailable);
                samples_2k += samplesAvailable;
                total_samples += samplesAvailable;

                ALshort* tmp = (ALshort*) ptr;
                int sum = 0, avg_amplitude = 0;

                /* calculate the average amplitude of samples from current iteration*/
                for (ALint i = 0; i <= samplesAvailable * NO_OF_CHANNELS; i++)
                {
                    sum += abs((int) *tmp++);
                }

                avg_amplitude = sum / samplesAvailable;

                /* checking the amplitude/volume greater that threshold */
                if (avg_amplitude > m_threshold)
                {
                    /* when even we get high amplitude ie. someone is talking reset the timer */
                    m_timer->ResetTimer();
                    VC_MSG("Timer Reset");
                    if (!process_data)
                    {
                        /* send a start tag to audioprocessor device to begin processing the data */
                        Buffer* b = m_output->GetBuffer();
                        b->SetTag(TAG_START);
                        m_output->PushBuffer(b);
                        process_data = true;
                    }

                    VC_TRACE("amplitude:%d %ld", avg_amplitude, m_timer->GetTimePassed());
                }

                ptr += samplesAvailable * NO_OF_CHANNELS;

                /* when we have enough data captured send the buffer to audio processor */
                if (process_data && samples_2k > 2048)
                {
                    buf->SetSamples(samples_2k);
                    buf->SetSize(samples_2k * 2 * NO_OF_CHANNELS);
                    m_output->PushBuffer(buf);
                    samples_2k = 0;
                    buf = m_output->GetBuffer();
                    ptr = (ALshort*) buf->GetData();
                }
                else if (!process_data)
                {
                    total_samples = 0;
                    m_mutex.Lock();
                    m_cv.Wait(50);
                    m_mutex.Unlock();
                }

                /**
                 * If enough samples are captured send the break tag to process the data
                 * till now and get a new set of fresh data TODO: find the avg value to be check for
                 */
                if (total_samples > 15000 && (avg_amplitude < m_threshold * 3 / 8))
                {
                    Buffer* b = m_output->GetBuffer();
                    b->SetTag(TAG_BREAK);
                    m_output->PushBuffer(b);
                    total_samples = 0;
                }

                /**
                 * if there is time out send the end tag to complete the data processing
                 * and wait for the start tag
                 */
                if (m_timer->GetTimePassed() >= 800)
                {
                    VC_MSG("TimeOut");
                    if (process_data)
                    {
                        alcCaptureStop(m_capturedev);
                        Buffer* b = m_output->GetBuffer();
                        b->SetTag(TAG_END);
                        m_output->PushBuffer(b);
                    }
                    break;
                }
            }
            else /* if there are no samples wait 50 ms*/
            {
                m_mutex.Lock();
                m_cv.Wait(50);
                m_mutex.Unlock();
            }
        }
    }
}

/**
 * Starts the capture thread
 */
void CaptureDevice::StartCapture()
{

}

/**
 * Stops the capture thread
 */
void CaptureDevice::StopCapture()
{

}

/**
 * Gets the current capture device
 * @return capture device name
 */
const char* CaptureDevice::GetCaptureDevice()
{
    return (alcGetString(m_capturedev, ALC_CAPTURE_DEVICE_SPECIFIER));
}

/**
 * Gets the current playback device
 * @return playback device name
 */
const char* CaptureDevice::GetPlaybackDevice()
{
    return (alcGetString(m_playbackdev, ALC_CAPTURE_DEVICE_SPECIFIER));
}
