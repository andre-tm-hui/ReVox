#include "monitor.h"

Monitor::Monitor(device inputDevice,
                 device outputDevice,
                 int sampleRate,
                 int framesPerBuffer,
                 std::string dir,
                 std::queue<float> *inputQueue,
                 std::queue<float> *playbackQueue) : AudioStream(inputDevice, outputDevice, sampleRate, framesPerBuffer, dir)
{
    initialSetup = false;
    data.inputQueue = inputQueue;
    data.playbackQueue = playbackQueue;
    data.monitorMic = 0.f;
    data.monitorSamples = 0.f;

    // open and start the stream to record from
    this->err = Pa_OpenStream(
                &this->stream,
                input ? &this->inputParameters : NULL,
                output ? &this->outputParameters : NULL,
                this->sampleRate,
                this->framesPerBuffer,
                paClipOff,
                input && output ? monitorCallback : NULL,
                &this->data
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
