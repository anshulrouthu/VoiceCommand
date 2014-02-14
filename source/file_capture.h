/***********************************************************
voiceCommand 

  Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

  All rights reserved.

  This software is distributed on an "AS IS" BASIS, 
  WITHOUT  WARRANTIES OR CONDITIONS OF ANY KIND, either 
  express or implied.
***********************************************************/

/*
 * file_capture.h
 *
 *  Created on: Feb 8, 2014
 *      Author: anshul
 */

#ifndef FILE_CAPTURE_H_
#define FILE_CAPTURE_H_

#include "apipe.h"
#include "utils.h"

class FileCapture: public ADevice
{
public:
    FileCapture(std::string name);
    ~FileCapture();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual InputPort* Input(int portno);
    virtual OutputPort* Output(int portno);
    virtual VC_STATUS SendCommand(VC_CMD cmd);
    virtual VC_STATUS SetParameters(const InputParams* params)
    {
        return (VC_NOT_IMPLEMENTED);
    }

    virtual VC_STATUS GetParameters(OutputParams* params)
    {
        return (VC_NOT_IMPLEMENTED);
    }

    const char* c_str()
    {
        return (m_name.c_str());
    }
private:
    VC_STATUS WriteData();
    FILE* m_file;
    InputPort* m_input;
    OutputPort* m_output;
    std::string m_name;

};

#endif /* FILE_CAPTURE_H_ */
