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
#include <iostream>
#include <fstream>
#include <termios.h>
#include <unistd.h>
#include <sstream>
#include <stdarg.h>
#include <getopt.h>
#include <string.h>

void DebugSetLevel(int level);
int kbhit(void);
int getch(void);

/*
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

#define DBG_ALWAYS 0
#define DBG_MESSAGE 1
#define DBG_TRACE 2

#define DBGPRINT(level,msg) vcDebug(level).DebugPrint msg

#define DBG_PRINT(level, format, args...)  DBGPRINT(level, ("%-5d%s() - " format "\n", __LINE__, __FUNCTION__, ##args))

//inorder to use the following debug function needs to implement c_str() function

#define VC_DBG(level, format, args...)  DBGPRINT(level, ("%-5d%s::%s() - " format "\n", __LINE__, c_str(), __FUNCTION__, ##args))
#define VC_DBG_ERR(level, format, args...)  DBGPRINT(level, ("%-5d%s::%s() - " format " in %s\n", __LINE__, c_str(), __FUNCTION__, ##args, __FILE__))
#define VC_ERR(format, args...)  VC_DBG_ERR(DBG_ALWAYS,  format, ##args)
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
    VC_FAILURE=0,
    VC_SUCCESS,
    VC_NOT_IMPLEMENTED,
    VC_UNDEFINED
}VC_STATUS;

template <typename T>
void write(std::ofstream& stream, const T& t) {
  stream.write((const char*)&t, sizeof(T));
}

template <typename SampleType>
void writeWAVData(const char* outFile, SampleType* buf, size_t bufSize,
                  int sampleRate, short channels)
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
  stream.write((const char*)&bufSize, 4);
  stream.write((const char*)buf, bufSize);
}

#endif /* UTILS_H_ */
