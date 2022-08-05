#include "noisegenerator.h"

NoiseGenerator::NoiseGenerator(device outputDevice, int sampleRate) : AudioStream({-1, -1}, outputDevice, sampleRate, 2, "")
{
    initialSetup = false;
    // open and start the stream to record from
    this->err = Pa_OpenStream(
                &this->stream,
                NULL,
                output ? &this->outputParameters : NULL,
                this->sampleRate,
                2,
                paClipOff,
                output ? noiseCallback : NULL,
                nullptr
                );

    if (this->err != paNoError) {
        std::cout<<Pa_GetErrorText(this->err)<<std::endl;
        done(); return;
    }
    streamSetup = true;

    this->err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cout<<Pa_GetErrorText(this->err)<<std::endl;
        done(); return;
    }
    initialSetup = true;
}
