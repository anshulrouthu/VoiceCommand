#include "audio_processor.h"

AudioProcessor::AudioProcessor()
{
    m_flac = new FLACWrapper();
    m_flac->init();
}

AudioProcessor::~AudioProcessor()
{
    delete m_flac;
}

char* AudioProcessor::ProcessAudioData(void* data,int samples)
{
    FILE* fp;
    char *cmd = (char*)malloc(5*1024);
    m_flac->createFLAC(data,samples);
    fp = popen("./bin/voiceRecord1","r");
    VC_CHECK(fp == NULL,,"Error Reading command");
    fscanf(fp, "\"%[^\"\n]\"\n", cmd);
    fclose (fp);
    VC_ALL("Received Command: %s",cmd);
    return (cmd);
}
