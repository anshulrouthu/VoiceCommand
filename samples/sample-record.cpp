#include "sample-record.h"

static void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data);

#define READSIZE 2048

static unsigned total_samples = 0; /* can use a 32-bit number due to WAVE size limitations */
static FLAC__byte* buffer;//[READSIZE/*samples*/ * 2/*bytes_per_sample*/ * 2/*channels*/]; /* we read the WAVE data into here */
static FLAC__int32 pcm[READSIZE/*samples*/ * 2/*channels*/];

int main(void)
{
    Timer t;
    /*
     * flac data
     */
    FLAC__bool ok = true;
    FLAC__StreamEncoder *encoder = 0;
    FLAC__StreamEncoderInitStatus init_status;
    FLAC__StreamMetadata *metadata[2];
    FLAC__StreamMetadata_VorbisComment_Entry entry;
    FILE *fin;
    unsigned sample_rate = 0;
    unsigned channels = 0;
    unsigned bps = 0;
//flac data end

    const ALCchar * devices;
    ALCdevice * mainDev;
    ALCcontext * mainContext;
    ALCdevice * captureDev;
    void * captureBuffer=(void*)malloc(1048576);
    ALshort *captureBufPtr;
    ALint samplesAvailable;
    ALint samplesCaptured;
    time_t currentTime;
    time_t lastTime;
    ALuint buf;
    ALuint source;
    ALint playState;
    int i;

// Print the list of capture devices
    //printf("Available playback devices:\n");

    //devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    //ptr = devices;
//while (ptr[0] != NULL)
    //while (*ptr)
    {
        //printf("   %s\n", ptr);
        //ptr += strlen(ptr) + 1;
    }

// Open a playback device and create a context first
    printf("Opening playback device:\n");
    mainDev = alcOpenDevice(NULL);
    if (mainDev == NULL)
    {
        printf("Unable to open playback device!\n");
        exit(1);
    }
    devices = alcGetString(mainDev, ALC_DEVICE_SPECIFIER);
    printf("   opened device '%s'\n", devices);
    mainContext = alcCreateContext(mainDev, NULL);
    if (mainContext == NULL)
    {
        printf("Unable to create playback context!\n");
        exit(1);
    }
    printf("   created playback context\n");

// Make the playback context current
    alcMakeContextCurrent(mainContext);
    alcProcessContext(mainContext);

// Print the list of capture devices

    //printf("Available capture devices:\n");
    //devices = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
    //ptr = devices;

//while (ptr[0] != NULL)
    //while (*ptr)
    {
        //printf("   %s\n", ptr);
        //ptr += strlen(ptr) + 1;
    }

// Open the default device
    printf("Opening capture device:\n");
    captureDev = alcCaptureOpenDevice(NULL, 16000, AL_FORMAT_STEREO16, 800);
    if (captureDev == NULL)
    {
        printf("   Unable to open device!\n");
        exit(1);
    }
    devices = alcGetString(captureDev, ALC_CAPTURE_DEVICE_SPECIFIER);
    printf("   opened device %s\n", devices);

// Wait for three seconds to prompt the user
    for (i = 2; i > 0; i--)
    {
        printf("Starting capture in %d...\r", i);
        fflush (stdout);
        lastTime = time(NULL);
        currentTime = lastTime;
        while (currentTime == lastTime)
        {
            currentTime = time(NULL);
            usleep(100000);
        }
    }

    printf("Starting capture NOW!\n");
    fflush (stdout);
    lastTime = currentTime;

// Capture (roughly) five seconds of audio
    alcCaptureStart(captureDev);
    samplesCaptured = 0;
    captureBufPtr = (ALshort*)captureBuffer;
    int sum=0,j;
    ALshort* tmp;
    t.StartTimer();
    while (!kbhit())
    {
        //t.StartTimer();
        printf("seconds passed: %d\n",t.GetTimePassed());
        // Get the number of samples available
        alcGetIntegerv(captureDev, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);

        // Copy the samples to our capture buffer
        if (samplesAvailable > 0)
        {
            alcCaptureSamples(captureDev, captureBufPtr, samplesAvailable);
            samplesCaptured += samplesAvailable;
            //printf("Captured %d samples (adding %d)\r", samplesCaptured, samplesAvailable);
            //fflush(stdout);

            // Advance the buffer (two bytes per sample * number of samples)
            captureBufPtr += samplesAvailable * 2;
        }

        if(j/100)
        {
            fflush(stdout);
            //if(sum)
                //printf("amplitude:%d\n",sum/j);
            tmp = captureBufPtr;
            sum = 0;
            j=0;
        }
        sum+=abs((int)*tmp++);
        j++;

        //if(sum/j>1000)
            //break;
        // Wait for a bit
        //usleep(10000);

        // Update the clock
        currentTime = time(NULL);
    }

    printf("\nDone capturing.\n");
    alcCaptureStop(captureDev);

// Play back the captured data
    printf("Starting playback...\n");
    fflush(stdout);

// Generate an OpenAL buffer for the captured data
    alGenBuffers(1, &buf);
    alGenSources(1, &source);
    alBufferData(buf, AL_FORMAT_STEREO16,(ALshort*)captureBuffer, samplesCaptured * 2, 16000);
    alSourcei(source, AL_BUFFER, buf);
    alSourcePlay(source);

    writeWAVData("audio.wav",(ALshort*)captureBuffer,samplesCaptured * 2, 16000, 2 );

    printf("samplescaptured: %d \n",samplesCaptured);

    if((fin = fopen("audio.wav", "rb")) == NULL) {
        fprintf(stderr, "ERROR: opening %s for output\n", "audio.wav");
        return 1;
    }

    sample_rate = 16000;
    channels = 2;
    bps = 16;
    total_samples = samplesCaptured;

    printf("sample_rate: %d %d\n",sample_rate,total_samples);

    /* allocate the encoder */
    if((encoder = FLAC__stream_encoder_new()) == NULL) {
        fprintf(stderr, "ERROR: allocating encoder\n");
        fclose(fin);
        return 1;
    }

    ok &= FLAC__stream_encoder_set_verify(encoder, true);
    ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
    ok &= FLAC__stream_encoder_set_channels(encoder, channels);
    ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bps);
    ok &= FLAC__stream_encoder_set_sample_rate(encoder, sample_rate);
    ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, total_samples);

    /* now add some metadata; we'll add some tags and a padding block */
    if(ok) {
        if(
            (metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) == NULL ||
            (metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL ||
            /* there are many tag (vorbiscomment) functions but these are convenient for this particular use: */
            !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", "Some Artist") ||
            !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) || /* copy=false: let metadata object take control of entry's allocated string */
            !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "YEAR", "1984") ||
            !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false)
        ) {
            fprintf(stderr, "ERROR: out of memory or tag error\n");
            ok = false;
        }

        metadata[1]->length = 1234; /* set the padding length */

        ok = FLAC__stream_encoder_set_metadata(encoder, metadata, 2);
    }

    /* initialize encoder */
    if(ok) {
        init_status = FLAC__stream_encoder_init_file(encoder, "audio.flac", progress_callback, /*client_data=*/NULL);
        if(init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
            fprintf(stderr, "ERROR: initializing encoder: %s\n", FLAC__StreamEncoderInitStatusString[init_status]);
            ok = false;
        }
    }

    /* read blocks of samples from WAVE file and feed to encoder */
    if(ok) {
        buffer = (FLAC__byte*)captureBuffer;
        size_t left = (size_t)total_samples;
        while(ok && left) {
            size_t need = (left>READSIZE? (size_t)READSIZE : (size_t)left);
            {
                /* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
                size_t i;
                for(i = 0; i < need*channels; i++) {
                    /* inefficient but simple and works on big- or little-endian machines */
                    pcm[i] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)buffer[2*i+1] << 8) | (FLAC__int16)buffer[2*i]);
                }
                /* feed samples to encoder */
                ok = FLAC__stream_encoder_process_interleaved(encoder, pcm, need);
            }

            left -= need;
            buffer+=need*4;
        }

    }

    ok &= FLAC__stream_encoder_finish(encoder);

    fprintf(stderr, "encoding: %s\n", ok? "succeeded" : "FAILED");
    fprintf(stderr, "   state: %s\n", FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);

    /* now that encoding is finished, the metadata can be freed */
    FLAC__metadata_object_delete(metadata[0]);
    FLAC__metadata_object_delete(metadata[1]);

    FLAC__stream_encoder_delete(encoder);


// Wait for the source to stop playing
    playState = AL_PLAYING;
    while (playState == AL_PLAYING)
    {
        printf("  source %d is playing...\r", source);
        fflush(stdout);
        alGetSourcei(source, AL_SOURCE_STATE, &playState);
        usleep(100000);
    }
    printf("\nDone with playback.\n");
    fflush(stdout);

// Shut down OpenAL
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buf);
    alcMakeContextCurrent (NULL);
    alcCloseDevice(mainDev);
    alcCaptureCloseDevice(captureDev);
}

void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
    (void)encoder, (void)client_data;

    fprintf(stderr, "wrote %llu bytes, %llu/%u samples, %u/%u frames\n", bytes_written, samples_written, total_samples, frames_written, total_frames_estimate);
}
