#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <string.h>
#include <iostream>
#include <chrono>
#include <map>
#include "portaudio.h"
#include "sndfile.h"
#include "../../audiofx/reverberator.h"
#include "../../audiofx/autotuner.h"
#include "../../audiofx/repitcher.h"

typedef struct{
    SNDFILE *file;
    SF_INFO info;
    bool pad;
    float timeStamp;
    bool inUse;
} recordData;

typedef struct{
    int timeLeft;
    float volume;
} soundData;

typedef struct{
    std::map<SNDFILE*, soundData> timers;
    std::vector<SNDFILE*> *queue;
    SF_INFO info;
    float maxFileLength;
    float *buf;
} playData;

typedef struct{
    recordData *rData;
    int nChannels;
    revmodel *reverb;
    PitchShift *autotune;
    PitchShift *pitchShift;
    float *buf;
    std::unordered_map<std::string, std::shared_ptr<IAudioFX>> *fxs;
} passthroughData;

typedef struct{
    recordData *rData;
    float monitorMic;
    float *inputBuffer;
    float monitorSamples;
    float *playbackBuffer;
} monitorData;

int playCallback(const void* inputBuffer, void* outputBuffer,
                 unsigned long framesPerBuffer,
                 const PaStreamCallbackTimeInfo* timeInfo,
                 PaStreamCallbackFlags statusFlags,
                 void* data);

int passthroughCallback(const void* inputBuffer, void* outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void* data);

int monitorCallback(const void* inputBuffer, void* outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void* data);

int streamCallback(const void* inputBuffer, void* outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void* data);


int noiseCallback(const void* inputBuffer, void* outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo* timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void* data);

#endif // CALLBACKS_H
