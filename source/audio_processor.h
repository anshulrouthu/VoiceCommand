#include "utils.h"
#include "flac.h"
#include "timer.h"
#include <json/json.h>
#include "curl/curl.h"
#include "buffer.h"

#define VC_SPEECH_ENGINE "https://www.google.com/speech-api/v1/recognize?xjerr=1&client=chromium&pfilter=0&maxresults=1&lang=\"en-US\""
#define VC_AUDIO_FILENAME "audio.flac"

class CURLWrapper
{
public:
    CURLWrapper(char* filename);
    ~CURLWrapper();
    char* GetText();
    static size_t WriteData(void *buffer, size_t size, size_t n, void *ptr);

private:
    const char* c_str()
    {
        return ("CURLWrapper");
    }
    CURL* m_curl;
    struct curl_slist *m_header;
    char* m_buffer;
    struct curl_httppost* m_formpost;
};

class AudioProcessor: public WorkerThread
{
public:
    AudioProcessor();
    virtual ~AudioProcessor();
    VC_STATUS ProcessAudioData(Buffer* buf);
    VC_STATUS PushBuffer(Buffer* buf);
    VC_STATUS InitiateDataProcessing();
    VC_STATUS CloseDataProcessing(char* text);
    VC_STATUS GetText(char* text);
    Buffer* GetBuffer();
    VC_STATUS RecycleBuffer(Buffer* buf);

private:
    virtual const char* c_str()
    {
        return ("AudioProcessor");
    }
    virtual void Task();
    FLACWrapper* m_flac;
    CURLWrapper* m_curl;
    Json::Reader m_reader;
    std::list<Buffer*> m_buffers;
    std::list<Buffer*> m_processbuf;
    char m_text[4*1024];
};
