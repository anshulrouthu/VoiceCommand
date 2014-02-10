/*
 * test_pipe.cpp
 *
 *  Created on: Feb 6, 2014
 *      Author: anshul
 */

#include "utils.h"
#include <UnitTest++.h>
#include "apipe.h"
#include "file_capture.h"

class TestDevice: public ADevice
{
public:
    TestDevice(std::string name) :
        m_name(name),
        m_inport("Inputport 0",this),
        m_outport("Outport 0",this)
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
        DBGPRINT(DBG_ALWAYS,("Testing ADeviceAPIs\n"));
        TestDevice* src = new TestDevice("TestDevice Src");
        CHECK_EQUAL(src->c_str(),"TestDevice Src");
        CHECK_EQUAL(src->Initialize(),VC_SUCCESS);
        CHECK_EQUAL(src->Notify(NULL),VC_SUCCESS);
        CHECK(!!src->Input(0));
        CHECK(!!src->Output(0));
        CHECK_EQUAL(src->SendCommand(VC_CMD_START),VC_SUCCESS);

        delete src;
    }

    TEST(ADeviceConnections)
    {
        DBGPRINT(DBG_ALWAYS,("Testing ADeviceConnections\n"));
        APipe* pipe = new APipe("Pipe 0");
        TestDevice* src = new TestDevice("TestDevice Src");
        TestDevice* dst = new TestDevice("TestDevice Dst");

        CHECK_EQUAL(pipe->c_str(),"Pipe 0");
        CHECK_EQUAL(src->c_str(),"TestDevice Src");
        CHECK_EQUAL(dst->c_str(),"TestDevice Dst");

        CHECK_EQUAL(pipe->ConnectDevices(src,dst),VC_SUCCESS);
        CHECK_EQUAL(pipe->DisconnectDevices(src,dst),VC_SUCCESS);

        delete pipe;
        delete src;
        delete dst;
    }

    TEST(InputOutputPorts)
    {
        DBGPRINT(DBG_ALWAYS,("Testing InputOutputPorts\n"));
        APipe* pipe = new APipe("Pipe 0");
        InputPort* input = new InputPort("Input 0",NULL);
        OutputPort* output = new OutputPort("Output 0", NULL);

        CHECK_EQUAL(pipe->c_str(),"Pipe 0");
        CHECK_EQUAL(input->c_str(),"Input 0");
        CHECK_EQUAL(output->c_str(),"Output 0");

        CHECK_EQUAL(pipe->ConnectPorts(input,output),VC_SUCCESS);
        Buffer* buf = output->GetBuffer();
        CHECK(!!buf);
        CHECK_EQUAL(buf->WriteData((void*)"VoiceCommand",12),VC_SUCCESS);
        CHECK_EQUAL(output->PushBuffer(buf),VC_SUCCESS);
        Buffer* buf2 = input->GetFilledBuffer();
        CHECK(!!buf2);
        CHECK_EQUAL((char*)buf->GetData(),"VoiceCommand");
        CHECK_EQUAL(pipe->DisconnectPorts(input,output),VC_SUCCESS);

        delete pipe;
        delete input;
        delete output;
    }

    TEST(FileCaptureTEST)
    {
        DBGPRINT(DBG_ALWAYS,("Testing FileCaptureTEST\n"));
        FILE* fp;
        APipe* pipe = new APipe("Pipe 0");
        Buffer* b = new Buffer();
        FileCapture* dst = new FileCapture("FileCapture");
        OutputPort* output = new OutputPort("Output 0", NULL);

        dst->Initialize();

        CHECK_EQUAL(pipe->c_str(),"Pipe 0");
        CHECK_EQUAL(dst->c_str(),"FileCapture");
        CHECK_EQUAL(output->c_str(),"Output 0");

        CHECK_EQUAL(pipe->ConnectPorts(dst->Input(0),output),VC_SUCCESS);

        Buffer* buf = output->GetBuffer();
        CHECK(!!buf);
        CHECK_EQUAL(buf->WriteData((void*)"VoiceCommand",12),VC_SUCCESS);
        CHECK_EQUAL(output->PushBuffer(buf),VC_SUCCESS);
        CHECK_EQUAL(pipe->DisconnectPorts(dst->Input(0),output),VC_SUCCESS);
        delete dst;

        fp = fopen("FileCapture.pcm","rb");
        fread(b->GetData(),sizeof(char),12,fp);

        CHECK_EQUAL((char*)b->GetData(),"VoiceCommand");

        fclose(fp);
        delete b;
        delete pipe;
        delete output;

    }

}
int main()
{
    return (UnitTest::RunAllTests());
}
