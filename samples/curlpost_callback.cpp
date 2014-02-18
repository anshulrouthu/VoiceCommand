/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * curlport_callback.cpp
 *
 *  Created on: Feb 12, 2014
 *      Author: anshul
 */

/* An example source code that issues a HTTP POST and we provide the actual
 * data through a read callback.
 */
#include "utils.h"
#include <curl/curl.h>

#define VC_SPEECH_ENGINE "https://www.google.com/speech-api/v1/recognize?xjerr=1&client=chromium&pfilter=0&maxresults=1&lang=\"en-US\""
char data[20868];
#define USE_CHUNKED
//#define DISABLE_EXPECT

struct WriteThis
{
    const char *readptr;
    long sizeleft;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    struct WriteThis *pooh = (struct WriteThis *) userp;

    if (size * nmemb < 1)
        return 0;

    if (pooh->sizeleft > 0)
    {
        *(char *) ptr = pooh->readptr[0]; /* copy one single byte */
        pooh->readptr++; /* advance pointer */
        pooh->sizeleft--; /* less data left */
        return 1; /* we return 1 byte at a time! */
    }

    return 0; /* no more data left to deliver */
}

int main(int argc, char* argv[])
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *m_header = NULL;

    struct WriteThis pooh;

    FILE* fp = fopen("hellowhatsup.flac", "rb");
    fread(data, 20868, 1, fp);

    pooh.readptr = data;
    pooh.sizeleft = 20868; //(long) strlen(data);

    fclose(fp);
    /* In windows, this will init the winsock stuff */
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
        return 1;
    }

    /* get a curl handle */
    curl = curl_easy_init();
    m_header = curl_slist_append(m_header, "Content-type: audio/x-flac; rate=16000");
    if (curl)
    {
        /* First set the URL that is about to receive our POST. */
        curl_easy_setopt(curl, CURLOPT_URL, VC_SPEECH_ENGINE);
        //curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_header);
        /* Now specify we want to POST data */
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        /* we want to use our own read function */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

        /* pointer to pass to our read function */
        curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);

        /* get verbose debug output please */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        /*
         If you use POST to a HTTP 1.1 server, you can send data without knowing
         the size before starting the POST if you use chunked encoding. You
         enable this by adding a header like "Transfer-Encoding: chunked" with
         CURLOPT_HTTPHEADER. With HTTP 1.0 or without chunked transfer, you must
         specify the size in the request.
         */
#ifdef USE_CHUNKED
        {
            struct curl_slist *chunk = NULL;

            chunk = curl_slist_append(chunk, "Content-type: audio/x-flac; rate=16000");
            chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
            res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
            /* use curl_slist_free_all() after the *perform() call to free this
             list again */
        }
#else
        /* Set the expected POST size. If you want to POST large amounts of data,
         consider CURLOPT_POSTFIELDSIZE_LARGE */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, pooh.sizeleft);
#endif

#ifdef DISABLE_EXPECT
        /*
         Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue"
         header.  You can disable this header with CURLOPT_HTTPHEADER as usual.
         NOTE: if you want chunked transfer too, you need to combine these two
         since you can only set one list of headers with CURLOPT_HTTPHEADER. */

        /* A less good option would be to enforce HTTP 1.0, but that might also
         have other implications. */
        {
            struct curl_slist *chunk = NULL;

            chunk = curl_slist_append(chunk, "Expect:");
            res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
            /* use curl_slist_free_all() after the *perform() call to free this
             list again */
        }
#endif

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0;
}

