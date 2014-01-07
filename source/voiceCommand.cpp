//============================================================================
// Name        : googleAV.cpp
// Author      : anshul
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <sprec/sprec.h>
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
#include <curl/curl.h>
using namespace std;
using namespace boost;

string exec(string);
void speakResponse(string);
int processCommand(char*);
int searchQuery(const char*);
int curlInit();
int kbhit(void);
static int curlWriter(char *data, size_t size, size_t nmemb, string *buffer);
void replaceAll(string&, const string&, const string&);
char errorbuf[CURL_ERROR_SIZE];
CURL *hcurl;

string curlbuf;
inline float getVolume(string recordHW, string com_duration, bool nullout) {
	FILE *cmd;
	float vol = 0.0f;
	string run = "arecord -D ";
	run += recordHW;
	run += " -f cd -t wav -d ";
	run += com_duration;
	run += " -r 16000 tmp.wav";
	if (nullout)
		run += " 1>/dev/null 2>/dev/null";
	system(run.c_str());
	cmd = popen(
			"sox tmp.wav -n stats -s 16 2>&1 | awk '/^Max\\ level/ {print $3}'",
			"r");
	fscanf(cmd, "%f", &vol);
	fclose(cmd);
	return vol;
}

int main(int argc, char* argv[]) {

	//const char* lang = "en-US";
	const char* duration = "1";
	//struct sprec_wav_header *hdr;
	//struct sprec_server_response *resp;
	//char *flac_file_buf;
	//int flac_file_len;
	char *text="0000";
	char message[1024];
	message[0]='0';
	//double confidence;
	//int err;
	bool active = 0;
	FILE* cmd=NULL;
	float vol=0.0f;
	float volthresh = 3.5f;
	string audiosrc="plughw:0,0";
	int c;
	if((c=getopt(argc, argv, "h"))!=-1){
		switch(c){
		case 'h':
			audiosrc = "Headset";
			speakResponse("Listening via headset");
			break;
		}
	}
	while (1) {
		message[0]='0';
		vol = getVolume(audiosrc, duration, true);
		if (vol > volthresh) {
			cout << "vol:" << vol <<"\n";
			system(
					"flac tmp.wav -f --best --sample-rate 16000 -o tmp.flac 1>/dev/null 2>/dev/null");
			cmd =
					popen(
							"wget -O - -o /dev/null --post-file tmp.flac --header=\"Content-Type: audio/x-flac; rate=16000\" http://www.google.com/speech-api/v1/recognize?lang=en | sed -e 's/[{}]/''/g'| awk -v k=\"text\" '{n=split($0,a,\",\"); for (i=1; i<=n; i++) print a[i]; exit }' | awk -F: 'NR==3 { print $3; exit }'",
							"r");
			if (cmd == NULL)
				printf("ERROR\n");
			fscanf(cmd, "\"%[^\"\n]\"\n", message);
			fclose(cmd);
			text = message;
			cout<<message<<"\n";
			//listenCommand();
			//printf("text:%s\n",message);

		}
		if (text[0]!='0' && vol > volthresh) {
			//ration = "4";
			//string cmd ="mpg123 -q \"http://translate.google.com/translate_tts?tl=en&q=yes sir\"";
			//exec(cmd);
			if (!strcmp(text, "bye bye")||!strcmp(text, "bye")) {
				processCommand(text);
				break;
			}
			if (!active) {
				cout << "Listening...\n";
				speakResponse("Yes Sir");
				duration = "3";
				active = 1;
			} else {
				//printf("Confidence: %d%%\n", (int) (confidence * 100));
				if (!processCommand(text)) {
					cout << "Searching answer:\n";
					searchQuery(text);
					duration = "1";
					active = 0;
				}
			}
		} else {
			cout << "\nIdle..\t";
			duration = "1";
			active = 0;
		}
		if (kbhit()) {
			cout<<"kbhit";
			if (getchar() == 27) {
				cout << "Exit\n";
				break;
			}
		}

		/**
		 * Let's not fill up the entire /tmp folder
		 */

	}
	system("rm tmp.flac");
	system("rm tmp.wav");
	return 0;
}

int processCommand(char* text) {
	string cmd;
	string s;
	string line;
	string response;
	bool cmdFound = 0;
	ifstream file("command.conf", ios::in);
	if (!file.is_open()) {
		printf("Can't find config file!\n");
		exit(0);
	}
	while (getline(file, line)) {
		unsigned int loc = line.find("==");
		string tmp = line.substr(0, loc);
		if (tmp.compare(text) == 0) {
			cmdFound = 1;
			response = line.substr(loc + 2);
			if(response[0]!='.'){
				cout<<response<<endl;
				speakResponse(response);
			}
			else{
				s = exec(response);
				cout<<s<<endl;
				speakResponse(s);
			}
		}
	}

	file.close();
	return cmdFound;
}

void speakResponse(string text) {
	string cmd ="./speak \"";
	cmd+=text;
	cmd+="\" 1>/dev/null 2>/dev/null";
	system(cmd.c_str());
}
string exec(string cmd) {

	//string cmd="mpg123 -q \"http://translate.google.com/translate_tts?tl=en&q="+(string)text+"\"";
	//sprintf(cmd,"mpg123 -q http://translate.google.com/translate_tts?tl=en&q=%s",text);
	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe)
		return "ERROR";
	char buffer[128];
	std::string result = "";
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	pclose(pipe);
	return result;
}
int searchQuery(const char* q) {
	CURLcode cr;
	int debug=1;
	curlInit();
	//technically this should never happen now.
	if (!hcurl) {
		cout << "hcurl is NULL.  Did you forget to call Init()?\n";
		return -1;
	}

	string link = "https://www.wolframalpha.com/input/?i=";
	link += q;
	replaceAll(link, string(" "), string("%20"));
	//printf("link: %s\n",link.c_str());
	curl_easy_setopt(hcurl, CURLOPT_URL, link.c_str());
	curlbuf.clear();
	cr = curl_easy_perform(hcurl);
	if (cr != CURLE_OK) {
		cout << "curl() error on getting link: " << errorbuf << endl;
		return -3;
	}

	if (debug & 2)
		cout << "\nLink curlbuf: [" << curlbuf << "]\n";

	regex rexp("0200\\.push\\( \\{\"stringified\": \"(.+?)\",\"");
	cmatch m;
	if (regex_search(curlbuf.c_str(), m, rexp)) {
		string t = string(m[1]);
		replaceAll(t, "\\n", " ");
		replaceAll(t, "|", " ");
		replaceAll(t, "\\", " ");
		printf("%s\n", t.c_str());
		speakResponse(t);

		return 0;
	} else {
		cout << "Could not find answer. Try again.\n";
		speakResponse("Sorry");
		return -1;
	}

	return 0;
}

//little helper function
void replaceAll(string& str, const string& from, const string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

int curlInit() {
	hcurl = curl_easy_init();
	if (!hcurl)
		return -1;
	curl_easy_setopt(hcurl, CURLOPT_ERRORBUFFER, errorbuf);
	curl_easy_setopt(hcurl, CURLOPT_WRITEFUNCTION, curlWriter);
	curl_easy_setopt(hcurl, CURLOPT_WRITEDATA, &curlbuf);

	curl_easy_setopt(hcurl, CURLOPT_HEADER, 0);
	curl_easy_setopt(hcurl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(hcurl, CURLOPT_USERAGENT,
			"Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.2.8) Gecko/20100804 Gentoo Firefox/3.6.8");
	//curl_easy_setopt(hcurl, CURLOPT_COOKIEJAR, "cookie.txt");
	return 0;
}

int curlWriter(char *data, size_t size, size_t nmemb, string *buffer) {
	if (buffer != NULL) {
		buffer->append(data, size * nmemb);
		return size * nmemb;
	}
	return 0;
}

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
