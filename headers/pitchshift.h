#ifndef PITCHSHIFT_H
#define PITCHSHIFT_H

#include <vector>
#include <cstring>
#include <cmath>
#include <math.h>
#include <frequency.h>
#include <windowfunctions.h>

class PitchShift
{
public:
    PitchShift(int bufSize, int sampleRate);

    void repitch(float *buf, float scale, bool tune = false);

private:
    void add(float *buf);

    std::vector<std::vector<float>> getWindows(float *buffer, float period, std::vector<float> markers);

    void addToBuffer(std::vector<float> window, float marker);

    std::vector<float> resample(std::vector<float> input, float scale);
    // by resampling, if played back at the original sampling rate, the pitch will change, as does the length
    // in order to return to the original length, we can cut or overlap-add to return it to original length
    // case pitchshift down: cut end of resampled vector, adjust phases to match

private:
    Frequency* freq;
    int bufSize;
    float *inputs, *outputs;
    float period = 0.f, prevPeriod = 0.f;
    float factor = 0.f, prevFactor = 0.f;
    float marker = 0.f, lastMarker = 0.f;

    bool setupFlag = false;
    bool working = false;
};

const float TWELTH_ROOT_TWO = pow(2.f, 1.f / 12.f);

#endif // PITCHSHIFT_H
