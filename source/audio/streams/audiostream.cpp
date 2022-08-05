#include "audiostream.h"

/* The default/base portaudio stream object. All other portaudio stream objects (recorder, player, passthrough) inherit this class */

AudioStream::AudioStream(
        device inputDevice,
        device outputDevice,
        int sampleRate,
        int framesPerBuffer,
        std::string dir)
{
    // set some properties of the object
    this->sampleRate = sampleRate;
    this->framesPerBuffer = framesPerBuffer;
    this->dir = dir;
    this->inputDevice = inputDevice.id;
    this->outputDevice = outputDevice.id;

    // setup the stream parameters of each device
    if (inputDevice.id >= 0){
        setupDevice(&inputParameters, inputDevice, true);
    }
    if (outputDevice.id >= 0){
        setupDevice(&outputParameters, outputDevice, false);
    }

    initialSetup = true;
}

AudioStream::~AudioStream()
{
    while(!initialSetup){};
    done();
}


void AudioStream::setupDevice(PaStreamParameters* parameters, device device, bool isInput)
{
    parameters->device = device.id;
    parameters->channelCount = device.nChannels > 2 ? 2 : device.nChannels;
    parameters->sampleFormat = PA_SAMPLE_TYPE;
    parameters->suggestedLatency = isInput ? Pa_GetDeviceInfo(device.id)->defaultLowInputLatency : Pa_GetDeviceInfo(device.id)->defaultLowOutputLatency;
    parameters->hostApiSpecificStreamInfo = NULL;
    if (isInput) { input = true; } else { output = true; }
}

void AudioStream::done()
{
    // an error has occured somewhere, so we close the stream if available
    printf("Done called. Terminated.\n"); fflush(stdout);
    if (err != paNoError)
    {
        fprintf(stderr, "An error occured while using the portaudio stream\n");
        fprintf(stderr, "Error number: %d\n", err);
        fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
        err = 1;          /* Always return 0 or 1, but no other return codes. */
    }

    if (streamSetup)
    {
        Pa_CloseStream(stream);
    }

    initialSetup = true;
}
