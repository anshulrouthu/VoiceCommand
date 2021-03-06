/***********************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file utils.h is part of VoiceCommand project

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
***********************************************************/

/*
 * utils.h
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sstream>
#include <getopt.h>
#include <stdarg.h>
#include <string.h>
#include <list>

#ifndef NULL
#define NULL   ((void *) 0)
#endif

#define NO_OF_CHANNELS 2

void DebugSetLevel(int level);
int kbhit(void);
int getch(void);
std::string ItoString(int id);

/**
 * Debugging code: helps us in easy debugging of the application
 */
class vcDebug
{
public:
    vcDebug(int level);
    void DebugPrint(const char* formatString, ...) __attribute__((__format__(printf, 2, 3)));

private:
    bool isDebugOn();
    int m_level;
};

#define DBG_ALWAYS 1
#define DBG_ERROR 2
#define DBG_MESSAGE 3
#define DBG_TRACE 4

#define DBGPRINT(level,msg) vcDebug(level).DebugPrint msg

#define DBG_PRINT(level, format, args...)  DBGPRINT(level, ("%-5d%s() - " format "\n", __LINE__, __FUNCTION__, ##args))

//debug macros for functions that do not implement c_str()

#define VC_DBG_STATIC(level, format, args...)  DBGPRINT(level, ("%-5d%s() - " format "\n", __LINE__, __FUNCTION__, ##args))
#define VC_ALL_STATIC(format, args...)  VC_DBG_STATIC(DBG_ALWAYS,  format, ##args)
#define VC_TRACE_STATIC(format, args...)  VC_DBG_STATIC(DBG_TRACE,   format, ##args)
#define VC_MSG_STATIC(format, args...)  VC_DBG_STATIC(DBG_MESSAGE,   format, ##args)
#define VC_ERR_STATIC(format, args...)  VC_DBG_STATIC(DBG_ERROR,  format, ##args)
#define VC_CHECK_STATIC(condition, fail, msg, args...)  \
do                                      \
{                                       \
    if (condition)                      \
    {                                   \
        VC_ERR_STATIC(msg,##args);             \
        fail;                           \
    }                                   \
} while (0)

//To use the following debug macros, the class should implement c_str() method

#define VC_DBG(level, format, args...)  DBGPRINT(level, ("%-5d%s::%s() - " format "\n", __LINE__, c_str(), __FUNCTION__, ##args))
#define VC_DBG_ERR(level, format, args...)  DBGPRINT(level, ("%-5d%s::%s() - " format " : %s\n", __LINE__, c_str(), __FUNCTION__, ##args, __FILE__))
#define VC_ERR(format, args...)  VC_DBG_ERR(DBG_ERROR,  format, ##args)
#define VC_ALL(format, args...)  VC_DBG(DBG_ALWAYS,  format, ##args)
#define VC_MSG(format, args...)  VC_DBG(DBG_MESSAGE,   format, ##args)
#define VC_TRACE(format, args...)  VC_DBG(DBG_TRACE,   format, ##args)

#define VC_CHECK(condition, fail, msg, args...)  \
do                                      \
{                                       \
    if (condition)                      \
    {                                   \
        VC_ERR(msg,##args);             \
        fail;                           \
    }                                   \
} while (0)

//debugging code end

typedef enum
{

} VC_EVENT;

typedef enum
{
    VC_CMD_START = 0, VC_CMD_STOP
} VC_CMD;

typedef enum
{
    VC_CAPTURE_DEVICE = 0,
    VC_AUDIO_PROCESSOR,
    VC_FLAC_DEVICE,
    VC_CURL_DEVICE,
    VC_TEXT_PROCESSOR,
    VC_COMMAND_PROCESSOR,
    VC_FILESINK_DEVICE,
    VC_FILESRC_DEVICE

} VC_DEVICETYPE;

typedef enum
{
    VC_FAILURE = 0,
    VC_SUCCESS,
    VC_NOT_IMPLEMENTED,
    VC_UNDEFINED
} VC_STATUS;

typedef enum
{
    TAG_NONE = 0,
    TAG_START,
    TAG_BREAK,
    TAG_END,
    TAG_EOS
} BUF_TAG;

const char* ConvertTagToString(BUF_TAG tag);

template<typename T>
void write(std::ofstream& stream, const T& t)
{
    stream.write((const char*) &t, sizeof(T));
}

template<typename SampleType>
void writeWAVData(const char* outFile, SampleType* buf, size_t bufSize, int sampleRate, short channels)
{
    std::ofstream stream(outFile, std::ios::binary);
    stream.write("RIFF", 4);
    write<int>(stream, 36 + bufSize);
    stream.write("WAVE", 4);
    stream.write("fmt ", 4);
    write<int>(stream, 16);
    write<short>(stream, 1);                                        // Format (1 = PCM)
    write<short>(stream, channels);                                 // Channels
    write<int>(stream, sampleRate);                                 // Sample Rate
    write<int>(stream, sampleRate * channels * sizeof(SampleType)); // Byterate
    write<short>(stream, channels * sizeof(SampleType));            // Frame size
    write<short>(stream, 8 * sizeof(SampleType));                   // Bits per sample
    stream.write("data", 4);
    stream.write((const char*) &bufSize, 4);
    stream.write((const char*) buf, bufSize);
}

#endif /* UTILS_H_ */
