#ifndef STFT_H
#define STFT_H

#include <fftw3.h>
#include <windowfunctions.h>
#include <cstring>
#include <vector>

class STFT
{
public:
    STFT(int bufSize, int frameFactor = 4, int overlapFactor = 4);
    fftwf_complex** stft(float *input);
    std::vector<float> istft(fftwf_complex **input, float timeScale = 1.f);

    int getBufSize() { return bufSize; }
    void setBufSize(int bufSize);

    int getFrameFactor() { return frameFactor; }
    void setFrameFactor(int frameFactor);

    int getOverlapFactor() { return overlapFactor; }
    void setOverlapFactor(int overlapFactor);

    int getFrameSize() { return frameSize; }
    int getFFTSize() { return fftSize; }
    int getHopSize() { return hopSize; }
    int getNFrames() { return nFrames; }

private:
    void updateParams();

private:
    float *in, *out;
    fftwf_complex *fft;
    fftwf_plan forward, backward;

    int bufSize = 256;
    int frameFactor = 4;
    int overlapFactor = 4;
    int frameSize;
    int fftSize;
    int hopSize;
    int nFrames;
};

#endif // STFT_H
