#include "utils.h"
#include "flac.h"
#include "timer.h"

class AudioProcessor
{
public:
    AudioProcessor();
    ~AudioProcessor();
    char* ProcessAudioData(void* data,int samples);

private:
    const char* c_str()
    {
        return ("AudioProcessor");
    }
    FLACWrapper* m_flac;
};
