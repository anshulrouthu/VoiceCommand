#include "audio_processor.h"
#define NUM_OF_BUFFERS 128

#define OLD_METHOD_PROCESSING 0

AudioProcessor::AudioProcessor()
{
    m_flac = new FLACWrapper((char*)VC_AUDIO_FILENAME);
    m_curl = new CURLWrapper((char*)VC_AUDIO_FILENAME);

    for(int i=0;i<NUM_OF_BUFFERS;i++)
    {
        Buffer* buf = new Buffer();
        m_buffers.push_back(buf);
    }
#if not OLD_METHOD_PROCESSING
    start();
#endif
}

AudioProcessor::~AudioProcessor()
{
    stop();
    join();
    delete m_flac;
    delete m_curl;
    for(std::list<Buffer*>::iterator it = m_buffers.begin(); it != m_buffers.end() ; it++)
    {
        delete *it;
    }
    m_buffers.clear();
}

VC_STATUS AudioProcessor::InitiateDataProcessing()
{
    m_flac->InitiateFLACCapture();
    return (VC_SUCCESS);
}

VC_STATUS AudioProcessor::CloseDataProcessing(char* text)
{
#if not OLD_METHOD_PROCESSING
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
            VC_ALL("\n\tUtterance: %s\n\tConfidence: %f", utterance, confidence);
            return (VC_SUCCESS);
        }
    }
#else
    VC_ALL("buffers size %d",m_processbuf.size());
    while(m_processbuf.size()!=0);
    m_flac->CloseFLACCapture();
    strcpy(text,m_text);
    m_text[0] = '\0';
    VC_ALL("Got text is %s",m_text);

#endif
    return (VC_SUCCESS);
}

VC_STATUS AudioProcessor::ProcessAudioData(Buffer* buf)
{
    VC_MSG("Enter");

    //just call flac write data to file
    int frames = m_flac->WriteData(buf->GetData(), buf->GetSamples());
    RecycleBuffer(buf);
    if(frames>2)
    {
        //send the data to google save the text untill close capture is closes
    }

    return (VC_FAILURE);
}

VC_STATUS AudioProcessor::GetText(char* text)
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

        if (confidence > 0.4 && hypotheses["utterance"].isString())
        {
            utterance = hypotheses["utterance"].asCString();
            strcpy(text, utterance);
            cmd[0] = '\0';
            VC_TRACE("\n\tUtterance: %s\n\tConfidence: %f", utterance, confidence);
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
        if(m_processbuf.size()>0)
        {
            char text[2048] = "";
            int samples;
            VC_MSG("Thread running buffer size %d", m_processbuf.size());
            Buffer* buf =  m_processbuf.front();
            m_processbuf.pop_front();

            if(buf->GetTag() == TAG_START)
            {
                InitiateDataProcessing();
            }
            else if (buf->GetTag() == TAG_BREAK && senddata)
            {
                VC_ALL("GOT TAG_BREAK");
                CloseDataProcessing(text);

                strcat(m_text," ");
                strcat(m_text,text);
                VC_MSG("GotText %s",m_text);
                usleep(10000);
                InitiateDataProcessing();

                senddata = false;
            }
            else if (buf->GetTag() == TAG_END)
            {
                CloseDataProcessing(text);
                strcat(m_text," ");
                strcat(m_text,text);
                VC_MSG("GotText %s",m_text);
                m_text[0] = '\0';
                senddata = false;
            }
            else
            {
                senddata=true;
                VC_MSG("Buffer address %x",buf->GetData());
                samples = m_flac->WriteData(buf->GetData(), buf->GetSamples());
                RecycleBuffer(buf);
            }

            if (samples > 5000)
            {

                VC_MSG("samples written %d", samples);
                //GetText(text);
                //m_flac->InitiateFLACCapture();
            }
        }
        else
        {
            //wait condition
            //usleep(1000);
        }
    }
}
VC_STATUS AudioProcessor::PushBuffer(Buffer* buf)
{
    m_processbuf.push_back(buf);
    VC_TRACE("processbuffer size %d", m_processbuf.size());
    return (VC_SUCCESS);
}

Buffer* AudioProcessor::GetBuffer()
{
    while(m_buffers.size() == 0){VC_ALL("Low on Buffers");};
    Buffer* buf = m_buffers.front();
    m_buffers.pop_front();
    VC_TRACE("m_buffer address %x", (unsigned int)buf);
    return (buf);
}

VC_STATUS AudioProcessor::RecycleBuffer(Buffer* buf)
{
    buf->Reset();
    m_buffers.push_back(buf);
    return (VC_SUCCESS);
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
