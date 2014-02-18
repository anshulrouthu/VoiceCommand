/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * test_pipe.cpp
 *
 *  Created on: Feb 6, 2014
 *      Author: anshul
 */

#include "utils.h"
#include <UnitTest++.h>
#include "apipe.h"
#include "file_io.h"

class TestDevice: public ADevice
{
public:
    TestDevice(std::string name) :
        m_name(name),
        m_inport("Inputport 0", this),
        m_outport("Outport 0", this)
    {
    }

    virtual ~TestDevice()
    {
    }

    virtual VC_STATUS Initialize()
    {
        return (VC_SUCCESS);
    }

    virtual VC_STATUS Notify(VC_EVENT* evt)
    {
        return (VC_SUCCESS);
    }

    virtual InputPort* Input(int portno)
    {
        return (&m_inport);
    }

    virtual OutputPort* Output(int portno)
    {
        return (&m_outport);
    }

    virtual VC_STATUS SendCommand(VC_CMD cmd)
    {
        return (VC_SUCCESS);
    }

    virtual VC_STATUS SetParameters(const InputParams* params)
    {
        return (VC_SUCCESS);
    }

    virtual VC_STATUS GetParameters(OutputParams* params)
    {
        return (VC_SUCCESS);
    }

    const char* c_str()
    {
        return (m_name.c_str());
    }

private:
    std::string m_name;
    InputPort m_inport;
    OutputPort m_outport;

};

SUITE(APipeFrameworkTest)
{
TEST(ADeviceAPIs)
{
    DBGPRINT(DBG_ALWAYS, ("Testing ADeviceAPIs\n"));
    TestDevice* src = new TestDevice("TestDevice Src");
    CHECK_EQUAL(src->c_str(), "TestDevice Src");
    CHECK_EQUAL(src->Initialize(), VC_SUCCESS);
    CHECK_EQUAL(src->Notify(NULL), VC_SUCCESS);
    CHECK(!!src->Input(0));
    CHECK(!!src->Output(0));
    CHECK_EQUAL(src->SendCommand(VC_CMD_START), VC_SUCCESS);

    delete src;
}

TEST(ADeviceConnections)
{
    DBGPRINT(DBG_ALWAYS, ("Testing ADeviceConnections\n"));
    APipe* pipe = new APipe("Pipe 0");
    TestDevice* src = new TestDevice("TestDevice Src");
    TestDevice* dst = new TestDevice("TestDevice Dst");

    CHECK_EQUAL(pipe->c_str(), "Pipe 0");
    CHECK_EQUAL(src->c_str(), "TestDevice Src");
    CHECK_EQUAL(dst->c_str(), "TestDevice Dst");

    CHECK_EQUAL(pipe->ConnectDevices(src, dst), VC_SUCCESS);
    CHECK_EQUAL(pipe->DisconnectDevices(src, dst), VC_SUCCESS);

    delete pipe;
    delete src;
    delete dst;
}

TEST(InputOutputPorts)
{
    DBGPRINT(DBG_ALWAYS, ("Testing InputOutputPorts\n"));
    APipe* pipe = new APipe("Pipe 0");
    InputPort* input = new InputPort("Input 0", NULL);
    OutputPort* output = new OutputPort("Output 0", NULL);

    CHECK_EQUAL(pipe->c_str(), "Pipe 0");
    CHECK_EQUAL(input->c_str(), "Input 0");
    CHECK_EQUAL(output->c_str(), "Output 0");

    CHECK_EQUAL(!!output->GetBuffer(), !!NULL);
    CHECK_EQUAL(pipe->ConnectPorts(input, output), VC_SUCCESS);
    Buffer* buf = output->GetBuffer();
    CHECK(!!buf);
    CHECK_EQUAL(buf->WriteData((void* )"VoiceCommand", 12), VC_SUCCESS);
    CHECK_EQUAL(output->PushBuffer(buf), VC_SUCCESS);
    Buffer* buf2 = input->GetFilledBuffer();
    CHECK(!!buf2);
    CHECK_EQUAL((char* )buf2->GetData(), "VoiceCommand");
    CHECK_EQUAL(input->RecycleBuffer(buf2), VC_SUCCESS);
    CHECK_EQUAL(pipe->DisconnectPorts(input, output), VC_SUCCESS);

    delete pipe;
    delete input;
    delete output;
}

TEST(FileIOTEST)
{
    DBGPRINT(DBG_ALWAYS, ("Testing FileIOTEST\n"));
    APipe* pipe = new APipe("Pipe 0");
    FileSink* dst = new FileSink("FileSink", "FileSink.out");
    FileSrc* src = new FileSrc("FileSrc", "FileSink.out");
    OutputPort* output = new OutputPort("Output 0", NULL);
    InputPort* input = new InputPort("Input 0", NULL);

    dst->Initialize();
    src->Initialize();

    CHECK_EQUAL(pipe->c_str(), "Pipe 0");
    CHECK_EQUAL(dst->c_str(), "FileSink");
    CHECK_EQUAL(src->c_str(), "FileSrc");
    CHECK_EQUAL(output->c_str(), "Output 0");
    CHECK_EQUAL(input->c_str(), "Input 0");

    CHECK_EQUAL(pipe->ConnectPorts(dst->Input(0), output), VC_SUCCESS);
    CHECK_EQUAL(pipe->ConnectPorts(input, src->Output(0)), VC_SUCCESS);

    CHECK_EQUAL(dst->SendCommand(VC_CMD_START), VC_SUCCESS);

    Buffer* buf = output->GetBuffer();
    CHECK(!!buf);
    CHECK_EQUAL(buf->WriteData((void* )"VoiceCommand", 12), VC_SUCCESS);
    CHECK_EQUAL(output->PushBuffer(buf), VC_SUCCESS);
    CHECK_EQUAL(pipe->DisconnectPorts(dst->Input(0), output), VC_SUCCESS);
    delete dst;

    CHECK_EQUAL(src->SendCommand(VC_CMD_START), VC_SUCCESS);

    //wait untill a buffer is pushed but source device
    while (!input->IsBufferAvailable())
        ;

    CHECK(input->IsBufferAvailable());

    buf = input->GetFilledBuffer();
    CHECK(!!buf);

    CHECK_EQUAL(buf->GetTag(), TAG_START);
    CHECK_EQUAL(input->RecycleBuffer(buf), VC_SUCCESS);

    //wait untill a buffer is pushed but source device
    while (!input->IsBufferAvailable())
        ;

    CHECK(input->IsBufferAvailable());

    buf = input->GetFilledBuffer();
    CHECK(!!buf);
    CHECK_EQUAL((char* )buf->GetData(), "VoiceCommand");

    CHECK_EQUAL(src->SendCommand(VC_CMD_STOP), VC_SUCCESS);
    CHECK_EQUAL(pipe->DisconnectPorts(input, src->Output(0)), VC_SUCCESS);
    delete src;

    FILE* fp;
    char c[12];
    fp = fopen("FileSink.out", "rb");
    fread(c, sizeof(char), 12, fp);
    CHECK_EQUAL(c, "VoiceCommand");

    fclose(fp);
    delete pipe;
    delete output;
    delete input;

}

}
int main()
{
    return (UnitTest::RunAllTests());
}
