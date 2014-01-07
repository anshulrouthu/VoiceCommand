// Use the newer ALSA API

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

void savePcmFile(struct Wave *waveFile);

int main (int argc, char *argv[])
{
    int i;
    int err;
    char buf[128];
    unsigned int rate = 44100;
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;

    if ((err = snd_pcm_open (&capture_handle, "hw:0,0", SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
        fprintf (stderr, "cannot open audio device %s (%s)\n",
            argv[1],
            snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
    {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
            snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0)
    {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
            snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        fprintf (stderr, "cannot set access type (%s)\n",
            snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
    {
        fprintf (stderr, "cannot set sample format (%s)\n",
            snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0)
    {
        fprintf (stderr, "cannot set sample rate (%s)\n",
            snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0)
    {
        fprintf (stderr, "cannot set channel count (%s)\n",
            snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0)
    {
        fprintf (stderr, "cannot set parameters (%s)\n",
            snd_strerror (err));
        exit (1);
    }

    snd_pcm_hw_params_free (hw_params);

    if ((err = snd_pcm_prepare (capture_handle)) < 0)
    {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
            snd_strerror (err));
        exit (1);
    }


    FILE *file = fopen("pcm.dat", "wb");

    for (i = 0; i < 10; ++i)
    {
        if ((err = snd_pcm_readi (capture_handle, buf, 128)) != 128)
        {
            fprintf (stderr, "read from audio interface failed (%s)\n",
                snd_strerror (err));
            exit (1);
        }

        size_t written;

        if (file == NULL)
        {
            fprintf(stderr, "Cannot open file for writing.\n");
            exit(1);
        }

        //written = fwrite(buf, 128, 1, file);

        if (written < 1)
        {
            fprintf(stderr, "Writing to file failed, error %d.\n", written);
            exit(1);
        }
    }

    fclose(file);

    snd_pcm_close (capture_handle);
    return (0);
}

void savePcmFile(char *waveFile)
{
    FILE *file = fopen("pcm.dat", "wb");
    size_t written;

    if (file == NULL)
    {
        fprintf(stderr, "Cannot open file for writing.\n");
        exit(1);
    }

    written = fwrite(waveFile, 128, 1, file);
    fclose(file);

    if (written < 1)
        ;
    {
        fprintf(stderr, "Writing to file failed, error %d.\n", written);
        exit(1);
    }
}
#else
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(int argc, char *argv[])
{
        void saveWaveFile(struct Wave *waveFile);

        long loops;
        int rc;
        int size;
        snd_pcm_t *handle;
        snd_pcm_hw_params_t *params;
        unsigned int sampleRate = 44100;
        int dir;
        snd_pcm_uframes_t frames;
        char *buffer;
        char *device = (char*) "hw:0,0";
        //char *device = (char*) "default";

        printf("Capture device is %s\n", device);
        /* Open PCM device for recording (capture). */
        rc = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0);
        if (rc < 0)
        {
                fprintf(stderr, "Unable to open PCM device: %s\n", snd_strerror(rc));
                exit(1);
        }

        /* Allocate a hardware parameters object. */
        snd_pcm_hw_params_alloca(&params);

        /* Fill it in with default values. */
        snd_pcm_hw_params_any(handle, params);

        /* Set the desired hardware parameters. */

        /* Interleaved mode */
        snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

        /* Signed 16-bit little-endian format */
        snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

        /* Two channels (stereo) */
        snd_pcm_hw_params_set_channels(handle, params, 2);

        /* 44100 bits/second sampling rate (CD quality) */
        snd_pcm_hw_params_set_rate_near(handle, params, &sampleRate, &dir);

        /* Set period size to 32 frames. */
        frames = 32;
        snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

        /* Write the parameters to the driver */
        rc = snd_pcm_hw_params(handle, params);
        if (rc < 0)
        {
                fprintf(stderr, "Unable to set HW parameters: %s\n", snd_strerror(rc));
                exit(1);
        }

        /* Use a buffer large enough to hold one period */
        snd_pcm_hw_params_get_period_size(params, &frames, &dir);
        size = frames * 4; /* 2 bytes/sample, 2 channels */
        buffer = (char *) malloc(size);

        /* We want to loop for 5 seconds */
        snd_pcm_hw_params_get_period_time(params, &sampleRate, &dir);
        loops = 2000000 / sampleRate;

        while (loops > 0)
        {
                loops--;
                rc = snd_pcm_readi(handle, buffer, frames);
                if (rc == -EPIPE)
                {
                        /* EPIPE means overrun */
                        fprintf(stderr, "Overrun occurred.\n");
                        snd_pcm_prepare(handle);
                } else if (rc < 0)
                {
                        fprintf(stderr, "Error from read: %s\n", snd_strerror(rc));
                } else if (rc != (int)frames)
                {
                        fprintf(stderr, "Short read, read %d frames.\n", rc);
                }
                if (rc != size) fprintf(stderr, "Short write: wrote %d bytes and size is %d.\n", rc, size);
        }


        FILE *file = fopen("pcm.wav", "wb");
        size_t written;

        if (file == NULL)
        {
                fprintf(stderr, "Cannot open file for writing.\n");
                exit(1);
        }

        written = fwrite(buffer, sizeof(buffer), 1, file);
        fclose(file);

        if (written < 1);
        {
                fprintf(stderr, "Writing to file failed, error %d.\n", written);
                exit(1);
        }


        Wave wave;

        strcpy(wave.riffHeader.chunkId, "RIFF");
        wave.riffHeader.chunkSize = 36 + size;
        strcpy(wave.riffHeader.format, "WAVE");
        strcpy(wave.formatHeader.chunkId, "fmt");
        wave.formatHeader.chunkSize = 16;
        wave.formatHeader.format = 1; // PCM, other value indicates compression
        wave.formatHeader.numChannels = 2; // Stereo
        wave.formatHeader.sampleRate = sampleRate;
        wave.formatHeader.byteRate = sampleRate * 2 * 2;
        wave.formatHeader.align = 2 * 2;
        wave.formatHeader.bitsPerSample = 16;

        strcpy(wave.dataHeader.chunkId, "data");
        wave.dataHeader.chunkSize = size;
        wave.dataHeader.data = buffer;

        //saveWaveFile(&wave);

        snd_pcm_drain(handle);
        snd_pcm_close(handle);
        free(buffer);

        return 0;
}

void saveWaveFile(struct Wave *waveFile)
{
    FILE *file = fopen("test.wav", "wb");
    size_t written;

    if (file == NULL)
    {
        fprintf(stderr, "Cannot open file for writing.\n");
        exit(1);
    }

    written = fwrite(waveFile, sizeof(waveFile), 1, file);
    fclose(file);

    if (written < 1)
        ;
    {
        fprintf(stderr, "Writing to file failed, error %d.\n", written);
        exit(1);
    }
}
#endif
