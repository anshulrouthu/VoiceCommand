#include "audio_processor.h"

AudioProcessor::AudioProcessor()
{
    m_flac = new FLACWrapper();
}

AudioProcessor::~AudioProcessor()
{
    delete m_flac;
}

const char* AudioProcessor::ProcessAudioData(void* data,int samples)
{
    FILE* fp;
    char *cmd = (char*)malloc(20*1024);
    m_flac->createFLAC(data,samples);
    fp = popen("./bin/voiceRecord1","r");
    VC_CHECK(fp == NULL,,"Error Reading command");
    //fscanf(fp, "%[a-zA-Z ]\n", cmd);
    fgets(cmd,1000,fp);
    fclose (fp);

    Json::Value root;
    Json::Value hypotheses;
    m_reader.parse(cmd,root,true);
    hypotheses = root["hypotheses"][(unsigned int)(0)];
    const char* utterance = hypotheses["utterance"].asString().c_str();
    double confidence = hypotheses["confidence"].asDouble();
    if(confidence > 0.8)
        VC_ALL("\n\tUtterance: %s\n\tConfidence: %f",utterance,confidence);
    return (utterance);
}
