#ifndef PHASEVOCODER_H
#define PHASEVOCODER_H

#include "fftw3.h"
#include "windowfunctions.h"
#include <string.h>
#include <map>
#include <algorithm>
#include <math.h>
#include "pvdat.h"


class PhaseVocoder
{
public:
    PhaseVocoder();

    void Apply(float* buf);

    Note GetKey() { return this->key; }
    void SetKey(Note key);

    int GetBufSize() { return this->bufSize; }
    void SetBufSize(int bufSize);

    int GetSampleRate() { return this->sampleRate; }
    void SetSampleRate(int sampleRate);

    int GetOverlap() { return this->overlap; }
    void SetOverlap(int overlap);

    WindowType GetWindowType() { return this->windowType; }
    void SetWindowType(WindowType windowType);

private:
    float GetFreqScale(float* input);

    void ApplyWindow(float* frame, int size);
    std::vector<float> ApplyWindow(std::vector<float> frame);

    float interp(float a, float b, float d);

private:
    float *in, *out, *autocorr;
    float *zeroPadded;
    fftwf_complex *freqs, *autocorrFreqs;
    fftwf_plan pf, pb, pfAuto, pbAuto;
    int bufSize = 256;
    int frameSize = 64;
    int sampleRate = 48000;
    int overlap = 4;
    float freqPerBin;
    WindowType windowType = Hann;

    bool hardtune = true;

    Note key = C;
    std::vector<float> magnitudes;
    std::vector<float> phases;
    std::vector<float> notes;

    std::map<Note, float> noteFreqs =
    {
        {C, 261.63},
        {Cs, 277.18},
        {D, 293.66},
        {Ds, 311.13},
        {E, 329.63},
        {F, 349.23},
        {Fs, 369.99},
        {G, 392},
        {Gs, 415.3},
        {A, 440},
        {As, 466.16},
        {B, 493.88}
    };

    std::map<Note, std::vector<Note>> scales =
    {
        {C, {C, D, E, F, G, A, B}},
        {D, {D, E, Fs, G, A, B, Cs}},
        {E, {E, Fs, Gs, A, B, Cs, Ds}},
        {F, {F, G, A, As, C, D, E}},
        {G, {G, A, B, C, D, E, Fs}},
        {A, {A, B, Cs, D, E, Fs, Gs}},
        {B, {B, Cs, Ds, E, Fs, As, Gs}}
    };

};

#endif // PHASEVOCODER_H
