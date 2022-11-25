#ifndef PITCHSHIFT_H
#define PITCHSHIFT_H

#include <vector>
#include <cstring>
#include <cmath>
#include <math.h>
#include <samplerate.h>
#include "frequency.h"
#include "windowfunctions.h"


class PitchShift
{
public:
    PitchShift(int bufSize, int sampleRate);
    ~PitchShift()
    {
        delete freq;
        delete[] inputs;
        delete[] outputs;
    }

    void repitch(float *buf);

    void setAutotune(bool enabled);
    bool getAutotune();

    void setPitchshift(bool enabled);
    bool getPitchshift();

    void setPitchscale(float scale);
    float getPitchscale();

    void setNotes(std::vector<bool> notes);
    std::vector<bool> getNotes();

    void reset();

private:
    void add(float *buf);

    std::vector<std::vector<float>> getWindows(float *buffer, float period, std::vector<int> markers, float scale = 1.f);

    void addToBuffer(std::vector<float> window, int marker);

    std::vector<float> resample(std::vector<float> input, float scale);
    // by resampling, if played back at the original sampling rate, the pitch will change, as does the length
    // in order to return to the original length, we can cut or overlap-add to return it to original length
    // case pitchshift down: cut end of resampled vector, adjust phases to match

private:
    Frequency *freq;
    int bufSize, sampleRate;
    float *inputs, *outputs;
    float period = 0.f, prevPeriod = 0.f;
    float factor = 0.f, prevFactor = 0.f;
    float marker = 0.f, lastMarker = 0.f;

    bool setupFlag = false;
    bool autotune = false;
    bool pitchshift = false;
    float pitchscale = 1.f;

    bool validNotes[12] = {true, false, true, false, true, true, false, true, false, true, false, true};
};

const float TWELTH_ROOT_TWO = pow(2.f, 1.f / 12.f);

#endif // PITCHSHIFT_H
