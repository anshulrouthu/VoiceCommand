#include "audio_processor.h"
#define NUM_OF_BUFFERS 128

AudioProcessor::AudioProcessor(std::string name):
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
    m_flac = new FLACWrapper((char*)VC_AUDIO_FILENAME);
    m_curl = new CURLWrapper((char*)VC_AUDIO_FILENAME);
    m_input = new InputPort("Inputport 0",this);
    m_output = new OutputPort("Ouputport 0",this);

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
    //TODO: update the api to notify different type of events
    m_mutex.Lock();
    m_cv.Notify();
    m_mutex.Unlock();

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
    bool senddata=false;
    while(m_state)
    {
        if(m_input->IsBufferAvailable())
        {
            char text[2048] = "";
            Buffer* buf = m_input->GetFilledBuffer();

            if(buf->GetTag() == TAG_START)
            {
                InitiateDataProcessing();
            }
            else if (buf->GetTag() == TAG_BREAK && senddata)
            {
                VC_MSG("GOT TAG_BREAK");
                CloseDataProcessing(text);

                strcat(m_text," ");
                strcat(m_text,text);
                VC_ALL("GotText %s\n",m_text);
                usleep(5000);
                InitiateDataProcessing();

                senddata = false;
            }
            else if (buf->GetTag() == TAG_END)
            {
                VC_MSG("GOT TAG_END");
                CloseDataProcessing(text);
                strcat(m_text," ");
                strcat(m_text,text);
                VC_ALL("GotText %s\n",m_text);

                if (strcmp(m_text," "))
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
                senddata=true;
                m_flac->WriteData(buf->GetData(), buf->GetSamples());
            }

            m_input->RecycleBuffer(buf);
        }
        else
        {
            //wait condition
            while (!m_input->IsBufferAvailable() && m_state)
            {
                m_mutex.Lock();
                m_cv.Wait();
                m_mutex.Unlock();
            }
        }
    }
}

CURLWrapper::CURLWrapper(char* filename):m_header(NULL),m_formpost(NULL)
{
    curl_global_init(CURL_GLOBAL_ALL);
    m_curl = curl_easy_init();

    VC_CHECK(m_curl == NULL,,"Error Initializing Curl");
    m_header = curl_slist_append(m_header, "Content-type: audio/x-flac; rate=16000");
    m_buffer = (char*) malloc(400*sizeof(char));
}

CURLWrapper::~CURLWrapper()
{
    curl_easy_cleanup(m_curl);
    curl_slist_free_all(m_header);
    curl_formfree(m_formpost);
    if(m_buffer)
        free(m_buffer);
}

char* CURLWrapper::GetText()
{
    VC_MSG("Enter");
    struct curl_httppost *lastptr = NULL;
    curl_formadd(&m_formpost, &lastptr, CURLFORM_COPYNAME, "sendfile", CURLFORM_FILE, "audio.flac", CURLFORM_END);

    curl_easy_setopt(m_curl, CURLOPT_URL,VC_SPEECH_ENGINE);
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_header);
    curl_easy_setopt(m_curl, CURLOPT_HTTPPOST, m_formpost);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, CURLWrapper::WriteData);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, (void*)m_buffer);
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 0);
    VC_CHECK(curl_easy_perform(m_curl) != CURLE_OK,return (NULL),"Error requesting command");
    return (m_buffer);
}

size_t CURLWrapper::WriteData(void* buffer,size_t size, size_t n, void* ptr)
{
    DBG_PRINT(DBG_TRACE,"Enter %d",size*n);
    //memcpy(ptr,buffer,size*n);
    strcpy((char*)ptr,(char*)buffer);
    DBG_PRINT(DBG_TRACE,"Command Data %s",(char*)buffer);
    return (size*n);
}
