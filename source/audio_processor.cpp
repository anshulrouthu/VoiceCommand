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

AudioProcessor::AudioProcessor(std::string name, APipe* pipe) :
    ADevice(name, pipe),
    m_flac(NULL),
    m_curl(NULL),
    m_output(NULL)
{
}

AudioProcessor::~AudioProcessor()
{
    /* this mutex is required by the main thread waiting on a condition
     take the mutex wake up main thread and release the mutex */
    {
        AutoMutex automutex(&m_mutex);
        m_cv.Notify();
    }

    Join();

    m_flac->SendCommand(VC_CMD_STOP);
    m_curl->SendCommand(VC_CMD_STOP);

    m_pipe->DisconnectDevices(m_flac,m_curl);
    m_pipe->DisconnectDevices(m_curl,this,0,1);

    delete m_flac;
    delete m_curl;
    delete m_output;

    for (std::map<int, InputPort*>::iterator it = m_input_map.begin(); it != m_input_map.end(); it++)
    {
        delete (*it).second;
        (*it).second = NULL;
    }


}

/**
 * Initialize the device and get all the resources
 */
VC_STATUS AudioProcessor::Initialize()
{
    m_flac = new FLACDevice("FlacDevice");
    m_curl = new CURLDevice("CurlDevice");
    m_output = new OutputPort("Ouputport 0", this);

    for (int i = 0; i < 2; i++)
    {
        InputPort* m_input = new InputPort("Inputport_"+std::string((char*)&i), this);
        m_input_map[i] = m_input;
    }

    m_flac->Initialize();
    m_curl->Initialize();

    m_pipe->ConnectDevices(m_flac,m_curl);
    m_pipe->ConnectDevices(m_curl,this,0,1);

    m_flac->SendCommand(VC_CMD_START);
    m_curl->SendCommand(VC_CMD_START);

    return (VC_SUCCESS);
}

/**
 * Return the input port of the device
 * @return m_input
 */
InputPort* AudioProcessor::Input(int portno)
{
    return (m_input_map[portno]);
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

std::string AudioProcessor::JSONToText(Buffer* buf)
{
    Json::Value root;
    Json::Value hypotheses;
    std::string utterance;
    double confidence;
    char *cmd;

    cmd = (char*)buf->GetData();
    if (cmd)
    {
        VC_CHECK(!m_reader.parse(cmd, root, true), return (""), "Error parsing text");

        hypotheses = root["hypotheses"][(unsigned int) (0)];

        if (hypotheses["confidence"].isDouble())
        {
            confidence = hypotheses["confidence"].asDouble();
            VC_MSG("Confidence %f", confidence);
        }

        if (confidence > 0.7 && hypotheses["utterance"].isString())
        {
            utterance = hypotheses["utterance"].asString();
            cmd[0] = '\0';
            VC_MSG("\n\tUtterance: %s\n\tConfidence: %f", utterance.c_str(), confidence);
            return (utterance);
        }
    }

    return ("");
}

void AudioProcessor::Task()
{
    VC_MSG("Enter");
    bool text_break = false;
    std::string text = "";
    while (m_state)
    {
        for (std::map<int, InputPort*>::iterator it = m_input_map.begin(); it != m_input_map.end(); it++)
        {
            InputPort* input = (*it).second;
            if (input->IsBufferAvailable())
            {
                if ((*it).first == 0)
                {
                    //first input port connected to capture device
                    Buffer* buf = input->GetFilledBuffer();

                    switch (buf->GetTag())
                    {
                    case TAG_START:
                        m_flac->StartEncoder();
                        break;
                    case TAG_BREAK:
                        text_break = true;
                        m_flac->StopEncoder();
                        m_flac->StartEncoder();
                        break;
                    case TAG_END:
                        text_break = false;
                        m_flac->StopEncoder();
                        break;
                    case TAG_NONE:
                        m_flac->WriteData(buf->GetData(), buf->GetSamples());
                        break;
                    default:
                        break;
                    }
                    input->RecycleBuffer(buf);
                }
                else
                {
                    //second input port connected to curldevice
                    Buffer* buf = input->GetFilledBuffer();
                    if(text_break)
                        text += " "+JSONToText(buf);
                    else
                        text = JSONToText(buf);
                    VC_ALL("Got Text: %s",text.c_str());
                    input->RecycleBuffer(buf);
                }
            }
            else
            {
                //wait condition
                AutoMutex automutex(&m_mutex);
                while (!IsBufferAvailable() && m_state)
                {
                    m_cv.Wait();
                }
            }
        }
    }
}

bool AudioProcessor::IsBufferAvailable()
{
    for (std::map<int, InputPort*>::iterator it = m_input_map.begin(); it != m_input_map.end(); it++)
    {
        if((*it).second->IsBufferAvailable())
        {
            return (true);
        }
    }
    return (false);
}
