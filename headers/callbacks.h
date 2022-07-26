#ifndef CALLBACKS_H
#define CALLBACKS_H

#endif // CALLBACKS_H

#include <string.h>
#include <iostream>
#include <chrono>
#include <map>
#include "portaudio.h"
#include "sndfile.h"
#include "revmodel.h"
#include "phasevocoder.h"

typedef struct{
    SNDFILE* file;
    std::map<SNDFILE*, int>* files;
    SF_INFO info;
    bool pad;
    float timeStamp;
    float maxFileLength;
    bool useReverb;
    revmodel* reverb;
    bool useAutotune;
    PhaseVocoder* pv;
} callbackData;


int recordCallback(const void* inputBuffer, void* outputBuffer,
                   unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo* timeInfo,
                   PaStreamCallbackFlags statusFlags,
                   void* data);

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
