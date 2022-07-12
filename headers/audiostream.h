#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#include "callbacks.h"
#include <filesystem>
#include <cstdint>
#include <fstream>

#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"

typedef struct{
    int id;
    int nChannels;
} device;

class AudioStream
{
public:
    AudioStream(device inputDevice, device outputDevice, int sampleRate, int framesPerBuffer, std::string dir);
    ~AudioStream();

    int inputDevice, outputDevice, sampleRate, sampleSize, framesPerBuffer;
    void done();

    PaStreamParameters inputParameters, outputParameters;
    PaStream* stream;
    bool streamSetup = false;
    PaError err = paNoError;

    void setupDevice(PaStreamParameters* parameters, device device, bool isInput = true);

    std::string dir;

    callbackData data = {};
};

#endif // AUDIOSTREAM_H
