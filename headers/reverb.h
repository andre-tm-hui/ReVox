#ifndef REVERB_H
#define REVERB_H

#include "comb.h"
#include "allpass.h"
#include "revtuning.h"

class Reverb
{
public:
    Reverb();

    void	process(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip);
    void	setroomsize(float value);
    float	getroomsize();
    void	setdamp(float value);
    float	getdamp();
    void	setwet(float value);
    float	getwet();
    void	setdry(float value);
    float	getdry();
    void	setwidth(float value);
    float	getwidth();
    void	setmode(float value);
    float	getmode();

    /*void Apply(float *buffer);

    void SetDelay(float delay);
    float decayFactor = 1;
    float dryWetMix = 0;
    int sampleRate = 48000;
    int bufferSize = 1;
    int nChannels = 2;

    void CombFilter(float *buffer, float *combBuffer, FIFOBuffer *fifoBuffer, float decayFactor);
    void AllPassFilter(float *buffer, float *outputBuffer, FIFOBuffer *allPassBuffer, FIFOBuffer *normalBuffer);*/
private:
    void update();

    float gain;
    float roomsize, roomsize1;
    float damp, damp1;
    float wet, wet1, wet2;
    float dry;
    float width;
    float mode;

    // Comb filters
    comb	combL[numcombs];
    comb	combR[numcombs];

    // Allpass filters
    allpass	allpassL[numallpasses];
    allpass	allpassR[numallpasses];

    // Buffers for the combs
    float	bufcombL1[combtuningL1];
    float	bufcombR1[combtuningR1];
    float	bufcombL2[combtuningL2];
    float	bufcombR2[combtuningR2];
    float	bufcombL3[combtuningL3];
    float	bufcombR3[combtuningR3];
    float	bufcombL4[combtuningL4];
    float	bufcombR4[combtuningR4];
    float	bufcombL5[combtuningL5];
    float	bufcombR5[combtuningR5];
    float	bufcombL6[combtuningL6];
    float	bufcombR6[combtuningR6];
    float	bufcombL7[combtuningL7];
    float	bufcombR7[combtuningR7];
    float	bufcombL8[combtuningL8];
    float	bufcombR8[combtuningR8];

    // Buffers for the allpasses
    float	bufallpassL1[allpasstuningL1];
    float	bufallpassR1[allpasstuningR1];
    float	bufallpassL2[allpasstuningL2];
    float	bufallpassR2[allpasstuningR2];
    float	bufallpassL3[allpasstuningL3];
    float	bufallpassR3[allpasstuningR3];
    float	bufallpassL4[allpasstuningL4];
    float	bufallpassR4[allpasstuningR4];
    /*FIFOBuffer *bufferA, *bufferB, *bufferC, *bufferD;
    FIFOBuffer *apA, *apB, *normalA, *normalB;
    float delay = 80;*/


};

#endif // REVERB_H
