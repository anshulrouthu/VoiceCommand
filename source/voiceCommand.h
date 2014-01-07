/*
 * voiceSimple.h
 *
 *  Created on: Jun 5, 2013
 *      Author: anshul
 */

#ifndef VOICESIMPLE_H_
#define VOICESIMPLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <boost/regex.hpp>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <termios.h>
#include <unistd.h>
#include <sstream>

#define DATA_SIZE 200
#define DURATION_DEFAULT "3"
#define COM_DURATION_DEFAULT "2"

using namespace std;

class VoiceCommand {
private:
	CURL *hcurl;
	CURLcode cr;
	bool use_pass, init;
	vector<string> voice, commands;
protected:
	string version;
public:
	bool continuous;
	bool verify;
	bool edit;
	bool ignoreOthers;
	bool filler;
	bool quiet;
	bool differentHW;
	float thresh;
	//I'm storing the durations as strings because it makes the commands less messy and requires less overhead
	string duration;
	string command_duration;
	string recordHW;
	string keyword;
	string config_file;
	string response;

	char errorbuf[CURL_ERROR_SIZE];
	string curlbuf;

	int debug;
	VoiceCommand();
	~VoiceCommand();

	inline void ProcessMessage(char* message);
	void GetConfig();
	void EditConfig();
	void CheckConfig();
	void CheckCmdLineParam(int argc, char* argv[]);
	void DisplayUsage();
	void Setup();

	int Search(const char* search);
	int Init(void);

	static int CurlWriter(char *data, size_t size, size_t nmemb,
			string *buffer);
};

#endif /* VOICESIMPLE_H_ */
