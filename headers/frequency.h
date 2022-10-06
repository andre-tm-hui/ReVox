#ifndef FREQUENCY_H
#define FREQUENCY_H

#define _USE_MATH_DEFINES
#include <fftw3.h>
#include <cmath>
//#include "windowfunctions.h"
#include <cstring>
#include <vector>

class Frequency
{
public:
    Frequency(float bufSize);
    ~Frequency()
    {
        delete[] in;
        delete[] out;
        delete[] fft;
    }

    float GetPeriod(float *buf);

private:


private:
    float *in, *out;
    fftwf_complex *fft;
    fftwf_plan forward, backward;

    int bufSize = 256;
    int inputSize;
    int fftSize;
};

#endif // FREQUENCY_H
