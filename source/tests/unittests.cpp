/*********************************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file unittests.cpp is part of VoiceCommand project

VoiceCommand is a free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

/*
 * test_pipe.cpp
 *
 *  Created on: Feb 6, 2014
 *      Author: anshul
 */

#include "utils.h"
#include "UnitTest++/UnitTest++.h"
#include "apipe.h"
#include "file_io.h"

class TestDevice: public ADevice
{
public:
    TestDevice(std::string name) :
        ADevice(name),
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

private:
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
    CHECK_EQUAL(src->SendCommand(VC_CMD_STOP), VC_SUCCESS);

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

    CHECK_EQUAL(pipe->DisconnectDevices(src, dst), VC_FAILURE);
    CHECK_EQUAL(pipe->ConnectDevices(src, NULL), VC_FAILURE);
    CHECK_EQUAL(pipe->ConnectDevices(NULL, NULL), VC_FAILURE);
    CHECK_EQUAL(pipe->ConnectDevices(NULL, dst), VC_FAILURE);
    CHECK_EQUAL(pipe->ConnectDevices(src, dst), VC_SUCCESS);
    CHECK_EQUAL(pipe->ConnectDevices(src, dst), VC_FAILURE);
    CHECK_EQUAL(pipe->DisconnectDevices(src, dst), VC_SUCCESS);
    CHECK_EQUAL(pipe->DisconnectDevices(dst, src), VC_FAILURE);
    CHECK_EQUAL(pipe->DisconnectDevices(NULL, NULL), VC_FAILURE);
    CHECK_EQUAL(pipe->ConnectDevices(src, dst), VC_SUCCESS);
    CHECK_EQUAL(pipe->DisconnectDevices(src, dst), VC_SUCCESS);

    delete pipe;
    delete src;
    delete dst;
}

TEST(InputOutputPorts)
{
    DBGPRINT(DBG_ALWAYS, ("Testing InputOutputPorts\n"));
    APipe* pipe = new APipe("InputOutput Pipe 0");
    InputPort* input = new InputPort("InputOutput Input 0", NULL);
    OutputPort* output = new OutputPort("InputOutput Output 0", NULL);

    CHECK_EQUAL(pipe->c_str(), "InputOutput Pipe 0");
    CHECK_EQUAL(input->c_str(), "InputOutput Input 0");
    CHECK_EQUAL(output->c_str(), "InputOutput Output 0");

    CHECK_EQUAL(!!output->GetBuffer(), !!NULL);
    CHECK_EQUAL(pipe->ConnectPorts(input, output), VC_SUCCESS);
    CHECK_EQUAL(pipe->ConnectPorts(input, output), VC_FAILURE);
    Buffer* buf = output->GetBuffer();
    CHECK(!!buf);
    CHECK_EQUAL(buf->WriteData((void* )"VoiceCommand", 12), VC_SUCCESS);
    CHECK_EQUAL(output->PushBuffer(buf), VC_SUCCESS);
    Buffer* buf2 = input->GetFilledBuffer();
    CHECK(!!buf2);
    CHECK(!memcmp(buf2->GetData(), "VoiceCommand", 12));
    CHECK_EQUAL(input->RecycleBuffer(buf2), VC_SUCCESS);
    CHECK_EQUAL(pipe->DisconnectPorts(input, output), VC_SUCCESS);
    CHECK_EQUAL(pipe->DisconnectPorts(input, output), VC_FAILURE);

    delete pipe;
    delete input;
    delete output;
}

TEST(FileIOTEST)
{
    DBGPRINT(DBG_ALWAYS, ("Testing FileIOTEST\n"));
    APipe* pipe = new APipe("FileIO Pipe 0");
    FileSink* fsink = new FileSink("FileIO FileSink", "FileSink.out");
    FileSrc* fsrc = new FileSrc("FileIO FileSrc", "FileSink.out");
    OutputPort* output = new OutputPort("FileIO Output 0", NULL);
    InputPort* input = new InputPort("FileIO Input 0", NULL);
    Buffer* buf;

    fsink->Initialize();
    fsrc->Initialize();

    CHECK_EQUAL(pipe->c_str(), "FileIO Pipe 0");
    CHECK_EQUAL(fsink->c_str(), "FileIO FileSink");
    CHECK_EQUAL(fsrc->c_str(), "FileIO FileSrc");
    CHECK_EQUAL(output->c_str(), "FileIO Output 0");
    CHECK_EQUAL(input->c_str(), "FileIO Input 0");

    CHECK_EQUAL(pipe->ConnectPorts(fsink->Input(0), output), VC_SUCCESS);
    CHECK_EQUAL(pipe->ConnectPorts(input, fsrc->Output(0)), VC_SUCCESS);

    CHECK_EQUAL(fsink->SendCommand(VC_CMD_START), VC_SUCCESS);
    usleep(100000); //wait for thread to start
    buf = output->GetBuffer();
    CHECK(!!buf);
    CHECK_EQUAL(buf->SetTag(TAG_START), VC_SUCCESS);
    CHECK_EQUAL(output->PushBuffer(buf), VC_SUCCESS);
    buf = output->GetBuffer();
    CHECK(!!buf);
    CHECK_EQUAL(buf->WriteData((void* )"VoiceCommand", 12), VC_SUCCESS);
    CHECK_EQUAL(output->PushBuffer(buf), VC_SUCCESS);
    buf = output->GetBuffer();
    CHECK(!!buf);
    CHECK_EQUAL(buf->SetTag(TAG_END), VC_SUCCESS);
    CHECK_EQUAL(output->PushBuffer(buf), VC_SUCCESS);
    CHECK_EQUAL(fsink->SendCommand(VC_CMD_STOP), VC_SUCCESS);
    CHECK_EQUAL(pipe->DisconnectPorts(fsink->Input(0), output), VC_SUCCESS);
    delete fsink;

    CHECK_EQUAL(fsrc->SendCommand(VC_CMD_START), VC_SUCCESS);

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
    CHECK(!memcmp(buf->GetData(), "VoiceCommand", 12));
    CHECK_EQUAL(fsrc->SendCommand(VC_CMD_STOP), VC_SUCCESS);
    CHECK_EQUAL(pipe->DisconnectPorts(input, fsrc->Output(0)), VC_SUCCESS);
    delete fsrc;

    FILE* fp;
    char c[12];
    fp = fopen("FileSink.out", "rb");
    int size = fread(c, 1, 12, fp);
    CHECK_EQUAL(size,12);
    CHECK(!memcmp(c, "VoiceCommand", 12));

    fclose(fp);
    delete pipe;
    delete output;
    delete input;

}

}
int main()
{
    DebugSetLevel(1);
    return (UnitTest::RunAllTests());
}
