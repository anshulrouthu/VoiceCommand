/***********************************************************
voiceCommand 

  Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

  All rights reserved.

  This software is distributed on an "AS IS" BASIS, 
  WITHOUT  WARRANTIES OR CONDITIONS OF ANY KIND, either 
  express or implied.
***********************************************************/

/*
 * sample-record.h
 *
 *  Created on: Dec 27, 2013
 *      Author: anshul
 */

#ifndef SAMPLE_RECORD_H_
#define SAMPLE_RECORD_H_

#include "utils.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <sys/time.h>
#include <ctime>
#include "FLAC/metadata.h"
#include "FLAC/stream_encoder.h"

#include <fstream>
#include "timer.h"

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

#endif /* SAMPLE_RECORD_H_ */
