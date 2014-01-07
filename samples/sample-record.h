/*
 * sample-record.h
 *
 *  Created on: Dec 27, 2013
 *      Author: anshul
 */

#ifndef SAMPLE_RECORD_H_
#define SAMPLE_RECORD_H_

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <sys/time.h>
#include <ctime>
#include "FLAC/metadata.h"
#include "FLAC/stream_encoder.h"

#include <fstream>


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

struct Riff
{
  char chunkId[4]; // "RIFF" (assuming char is 8 bits)
  int chunkSize; // (assuming int is 32 bits)
  char format[4]; // "WAVE"
};

struct Format
{
  char chunkId[4]; // "fmt "
  int chunkSize;
  short format; // assuming short is 16 bits
  short numChannels;
  int sampleRate;
  int byteRate;
  short align;
  short bitsPerSample;
};

struct Data
{
  char chunkId[4]; // "data"
  int chunkSize; // length of data
  char* data;
};

struct Wave // Actual structure of a PCM WAVE file
{
  Riff riffHeader;
  Format formatHeader;
  Data dataHeader;
};

int kbhit(void) {
    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);

    select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);

}


#endif /* SAMPLE_RECORD_H_ */
