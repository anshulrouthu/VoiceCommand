/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

#include "audio_processor.h"
#define NUM_OF_BUFFERS 128

AudioProcessor::AudioProcessor(std::string name) :
    m_cv(m_mutex),
    m_name(name)
{
}

AudioProcessor::~AudioProcessor()
{
    m_mutex.Lock();
    m_cv.Notify();
    m_mutex.Unlock();

    Join();

    delete m_flac;
    delete m_curl;

    delete m_input;
    delete m_output;

}

/**
 * Initialize the device and get all the resources
 */
VC_STATUS AudioProcessor::Initialize()
{
    m_flac = new FLACDevice((char*) VC_AUDIO_FILENAME);
    m_flac->Initialize();
    m_curl = new CURLDevice("CurlDevice");
    m_input = new InputPort("Inputport 0", this);
    m_output = new OutputPort("Ouputport 0", this);

    return (VC_SUCCESS);
}

/**
 * Return the input port of the device
 * @return m_input
 */
InputPort* AudioProcessor::Input(int portno)
{
    return (m_input);
}

/**
 * Return the output port of the device
 * @return m_output
 */
OutputPort* AudioProcessor::Output(int portno)
{
    return (m_output);
}

/**
 * Notifies the device of any event
 */
VC_STATUS AudioProcessor::Notify(VC_EVENT* evt)
{
    //TODO: update the api to notify different type of events, as needed
    AutoMutex automutex(&m_mutex);
    m_cv.Notify();

    return (VC_SUCCESS);
}

/**
 * Send a command to the device
 */
VC_STATUS AudioProcessor::SendCommand(VC_CMD cmd)
{
    switch (cmd)
    {
    case VC_CMD_START:
        Start();
        break;
    case VC_CMD_STOP:
        Stop();
        break;
    }
    return (VC_SUCCESS);
}

/**
 * Set the required parameters for device
 * @paran params
 */
VC_STATUS AudioProcessor::SetParameters(const InputParams* params)
{
    return (VC_SUCCESS);
}

/**
 * Get the required parameters from device
 * @paran params
 */
VC_STATUS AudioProcessor::GetParameters(OutputParams* params)
{
    return (VC_SUCCESS);
}

VC_STATUS AudioProcessor::InitiateDataProcessing()
{
    m_flac->InitiateFLACCapture();
    return (VC_SUCCESS);
}

VC_STATUS AudioProcessor::CloseDataProcessing(char* text)
{
    Json::Value root;
    Json::Value hypotheses;
    const char* utterance;
    double confidence;
    char *cmd;

    m_flac->CloseFLACCapture();
    text[0] = '\0';
    cmd = m_curl->GetText();
    if (cmd)
    {
        VC_CHECK(!m_reader.parse(cmd, root, true), return (VC_FAILURE), "Error parsing text");

        hypotheses = root["hypotheses"][(unsigned int) (0)];

        if (hypotheses["confidence"].isDouble())
        {
            confidence = hypotheses["confidence"].asDouble();
            VC_MSG("Confidence %f", confidence);
        }

        if (confidence > 0.7 && hypotheses["utterance"].isString())
        {
            utterance = hypotheses["utterance"].asCString();
            strcpy(text, utterance);
            cmd[0] = '\0';
            VC_MSG("\n\tUtterance: %s\n\tConfidence: %f", utterance, confidence);
            return (VC_SUCCESS);
        }
    }

    return (VC_SUCCESS);
}

void AudioProcessor::Task()
{
    VC_ALL("Enter");
    bool senddata = false;
    while (m_state)
    {
        if (m_input->IsBufferAvailable())
        {
            char text[2048] = "";
            Buffer* buf = m_input->GetFilledBuffer();

            if (buf->GetTag() == TAG_START)
            {
                InitiateDataProcessing();
            }
            else if (buf->GetTag() == TAG_BREAK && senddata)
            {
                VC_MSG("GOT TAG_BREAK");
                CloseDataProcessing(text);

                strcat(m_text, " ");
                strcat(m_text, text);
                VC_ALL("GotText %s\n", m_text);
                usleep(5000);
                InitiateDataProcessing();

                senddata = false;
            }
            else if (buf->GetTag() == TAG_END)
            {
                VC_MSG("GOT TAG_END");
                CloseDataProcessing(text);
                strcat(m_text, " ");
                strcat(m_text, text);
                VC_ALL("GotText %s\n", m_text);

                if (strcmp(m_text, " "))
                {
                    char notifycmd[4 * 1000] = "notify-send -t 10 \"Received Text\" \"";
                    strcat(notifycmd, m_text);
                    strcat(notifycmd, "\"");
                    //TODO:improve this GUI notification mechanism
                    system("pkill notify-osd");
                    system(notifycmd);
                }
                m_text[0] = '\0';
                senddata = false;
            }
            else
            {
                senddata = true;
                m_flac->WriteData(buf->GetData(), buf->GetSamples());
            }

            m_input->RecycleBuffer(buf);
        }
        else
        {
            //wait condition
            AutoMutex automutex(&m_mutex);
            while (!m_input->IsBufferAvailable() && m_state)
            {
                m_cv.Wait();
            }
        }
    }
}

