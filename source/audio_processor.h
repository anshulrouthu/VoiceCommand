#include "utils.h"
#include "flac.h"
#include "timer.h"
#include <json/json.h>

using namespace Json;

class AudioProcessor
{
public:
    AudioProcessor();
    ~AudioProcessor();
    const char* ProcessAudioData(void* data,int samples);

private:
    const char* c_str()
    {
        return ("AudioProcessor");
    }
    FLACWrapper* m_flac;
    Reader m_reader;
    Value  m_value;
};
