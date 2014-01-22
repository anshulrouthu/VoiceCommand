#include "audio_processor.h"

AudioProcessor::AudioProcessor()
{
    m_flac = new FLACWrapper((char*)VC_AUDIO_FILENAME);
    m_curl = new CURLWrapper((char*)VC_AUDIO_FILENAME);
}

AudioProcessor::~AudioProcessor()
{
    delete m_flac;
    delete m_curl;
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
            VC_TRACE("\n\tUtterance: %s\n\tConfidence: %f", utterance, confidence);
            return (VC_SUCCESS);
        }
    }
    return (VC_SUCCESS);
}

VC_STATUS AudioProcessor::ProcessAudioData(void* data, int samples)
{
    VC_MSG("Enter");

    //just call flac write data to file
    VC_CHECK(m_flac->WriteData(data, samples)!= VC_SUCCESS,,"Error creating FLAC file");

    return (VC_FAILURE);
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
