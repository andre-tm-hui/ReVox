#include "reverb.h"
#include <stdio.h>
#include <cmath>

Reverb::Reverb()
{
    bufferA = new FIFOBuffer();
    bufferB = new FIFOBuffer();
    bufferC = new FIFOBuffer();
    bufferD = new FIFOBuffer();

    apA = new FIFOBuffer();
    apB = new FIFOBuffer();
    apA->SetSize(this->bufferSize * this->nChannels + int(90 * (this->sampleRate / 1000)));
    apB->SetSize(this->bufferSize * this->nChannels + int(90 * (this->sampleRate / 1000)));

    normalA = new FIFOBuffer();
    normalB = new FIFOBuffer();
    normalA->SetSize(this->bufferSize * this->nChannels + int(90 * (this->sampleRate / 1000)));
    normalB->SetSize(this->bufferSize * this->nChannels + int(90 * (this->sampleRate / 1000)));
    SetDelay(this->delay);
}


void Reverb::SetDelay(float delay)
{
    this->delay = delay;
    // framesPerBuffer * numChannels = minimumBufferSize
    // add on (sampleRate / 1000 (samples per ms)) * delay
    bufferA->SetSize(this->bufferSize * this->nChannels + (int)(delay * (this->sampleRate / 1000)));
    bufferB->SetSize(this->bufferSize * this->nChannels + (int)((delay - 11.73) * (this->sampleRate / 1000)));
    bufferC->SetSize(this->bufferSize * this->nChannels + (int)((delay + 19.31) * (this->sampleRate / 1000)));
    bufferD->SetSize(this->bufferSize * this->nChannels + (int)((delay - 7.97) * (this->sampleRate / 1000)));
    fprintf(stdout, "reverber delay set\n"); fflush(stdout);
}

void Reverb::Apply(float *buffer)
{
    float *combA = new float[this->bufferSize * this->nChannels];
    float *combB = new float[this->bufferSize * this->nChannels];
    float *combC = new float[this->bufferSize * this->nChannels];
    float *combD = new float[this->bufferSize * this->nChannels];
    CombFilter(buffer, combA, bufferA, this->decayFactor);
    CombFilter(buffer, combB, bufferB, this->decayFactor - 0.1313f);
    CombFilter(buffer, combC, bufferC, this->decayFactor - 0.2743f);
    CombFilter(buffer, combD, bufferD, this->decayFactor - 0.31f);

    for (int i = 0; i < this->bufferSize * this->nChannels; i++)
    {
        buffer[i] = (this->dryWetMix * buffer[i])
                + ((1 - this->dryWetMix) * (combA[i] + combB[i] + combC[i] + combD[i]));
    }

    delete[] combA;
    delete[] combB;
    delete[] combC;
    delete[] combD;

    float *allPassA = new float[this->bufferSize * this->nChannels];
    AllPassFilter(buffer, allPassA, apA, normalA);
    AllPassFilter(allPassA, buffer, apB, normalB);

    delete[] allPassA;
}

void Reverb::AllPassFilter(float *buffer, float *outputBuffer, FIFOBuffer *allPassBuffer, FIFOBuffer *normalBuffer)
{
    float decayFactor = 0.15;

    for (int i = 0; i < this->bufferSize * this->nChannels; i++)
    {
        outputBuffer[i] = buffer[i];
        outputBuffer[i] += -decayFactor * allPassBuffer->buffer[0];
        outputBuffer[i] += decayFactor * allPassBuffer->buffer[20 * this->nChannels];
        allPassBuffer->Add(outputBuffer[i]);
    }

    float valueL = normalBuffer->buffer[normalBuffer->size - 2];
    float valueR = normalBuffer->buffer[normalBuffer->size - 1];
    for (int i = 0; i < this->bufferSize; i++)
    {
        float currentValueL = outputBuffer[2 * i];
        float currentValueR = outputBuffer[(2 * i) + 1];

        valueL = ((valueL + (currentValueL - valueL)) / allPassBuffer->max);
        valueR = ((valueR + (currentValueR - valueR)) / allPassBuffer->max);
        outputBuffer[2 * i] = valueL;
        outputBuffer[(2 * i) + 1] = valueR;
        normalBuffer->Add(outputBuffer[2 * i]);
        normalBuffer->Add(outputBuffer[(2 * i) + 1]);
        //fprintf(stdout, "l: %f, r: %f\n", valueL, valueR); fflush(stdout);
    }
}


void Reverb::CombFilter(float *buffer, float *combBuffer, FIFOBuffer *fifoBuffer, float decayFactor)
{
    for (int i = 0; i < this->bufferSize * this->nChannels; i++)
    {
        combBuffer[i] = buffer[i] + fifoBuffer->buffer[0] * decayFactor;
        fifoBuffer->Add(buffer[i]);
    }
}
