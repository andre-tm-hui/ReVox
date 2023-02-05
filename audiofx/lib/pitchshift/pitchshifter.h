#ifndef PITCHSHIFTER_H
#define PITCHSHIFTER_H
#define _USE_MATH_DEFINES

#include <thread>
#include <future>
#include <cmath>
#include <chrono>
#include <fftw3.h>
#include "frequency.h"
#include "../../../util/ringbuffer.h"

class PitchShifter
{
public:
    PitchShifter(int bufSize, int inputSize, int sampleRate);

    void process(float *buf);

    void setTargetPitch(float targetPitch);

    void setAutotuneActive(bool enabled) { autotune = enabled; }
    bool getAutotuneActive() { return autotune; }

    void setRepitchActive(bool enabled) { repitch = enabled; }
    bool getRepitchActive() { return repitch; }

    void setRepitchFactor(float factor) { repitchFactor = factor; }
    float getRepitchFactor() { return repitchFactor; }

    void setRetuneSpeed(int speed) { retuneSpeed = speed; }
    int getRetuneSpeed() { return retuneSpeed; }

    void setAutotuneNotes(std::vector<bool> notes) { validNotes = notes; }
    std::vector<bool> getAutotuneNotes() { return validNotes; }

    void reset();

private:
    void processRepitch(std::chrono::nanoseconds startTime);
    std::chrono::nanoseconds lastFrameTime = std::chrono::nanoseconds(0);

    float getTargetFreqFactor(float period);

    std::vector<float> getMarkers(float period, float *input);

    std::vector<float> synthesizeMarkers(float period, float factor, std::vector<float> markers);

    std::vector<std::vector<float>> getWindows(float *buffer, float period, std::vector<float> markers, float factor = 1.f);
    std::vector<float> getWindow(float *buf, float period, float marker, float factor);
    std::vector<float> resample(std::vector<float> input, float factor);

    void overlapAdd(std::vector<std::vector<float>> windows, std::vector<float> markers, std::vector<float> synthesizedMarkers);
    void addWindow(std::vector<float> window, float marker);

    void cleanup(float factor, float period);

    Frequency *freq;
    RingBuffer<float> input;
    float *output;
    int     outputPos = 0,
            framesWritten = 0,
    // editable settings
            retuneSpeed = 0,
            bufSize,
            inputSize,
            sampleRate,
    // carry-over fields
            lastMarker;
    float   prevPeriod,
            prevFactor,
            lastTargetSemitone,
            repitchFactor = 1.f;
    bool    setupFlag = false,
            autotune = false,
            repitch = false;

    std::vector<bool> validNotes = {true, false, true, false, true, true, false, true, false, true, false, true};
};

const float A_ONE = 55.f;
const float C_ONE = A_ONE * pow(2.f, 0.25f);

#endif // PITCHSHIFTER_H
