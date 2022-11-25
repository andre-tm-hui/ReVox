#include "pitchshift.h"
#include <iostream>

PitchShift::PitchShift(int bufSize, int sampleRate)
{
    this->bufSize = bufSize;
    this->sampleRate = sampleRate;
    freq = new Frequency(bufSize);
    inputs = new float[bufSize * 3];
    outputs = new float[bufSize * 3];
    memset(inputs, 0, sizeof(float) * bufSize * 3);
    memset(outputs, 0, sizeof(float) * bufSize * 3);

    lastMarker = (float)bufSize+1.f;
}

void PitchShift::setAutotune(bool enabled) { this->autotune = enabled; }
bool PitchShift::getAutotune() { return this->autotune; }

void PitchShift::setPitchshift(bool enabled) { this->pitchshift = enabled; }
bool PitchShift::getPitchshift() { return this->pitchshift; }

void PitchShift::setPitchscale(float scale) { this->pitchscale = scale; }
float PitchShift::getPitchscale() { return this->pitchscale; }

std::vector<bool> PitchShift::getNotes() {
    std::vector<bool> notes(std::begin(validNotes), std::end(validNotes));
    return notes;
}

void PitchShift::setNotes(std::vector<bool> notes)
{
    for (int i = 0; i < 12; i++) validNotes[i] = notes[i];
}

void PitchShift::repitch(float *buf)
{
    float scale = 1.f;
    // Add the buffer to the process loop
    add(buf);
    // Get the period of the buffer to be processed
    period = freq->GetPeriod(inputs + bufSize);

    if (period > 800 || period < 24)
    {
        setupFlag = false;
        lastMarker = (float)bufSize+1.f;

        if (prevPeriod != 0.f)
        {
            period = prevPeriod;
        }
        else
        {
            prevPeriod = period;
            prevFactor = scale;

            memcpy(buf, outputs, sizeof(float) * bufSize);
            return;
        }
    }

    if (pitchshift) scale *= pitchscale;

    if (autotune)
    {
        float ffreq = (float)sampleRate / period;
        float semitoneOffset = log(ffreq / 440.f) / log(TWELTH_ROOT_TWO);
        float modOffset = fmod(semitoneOffset, 12);
        int octaveOffset = (int)(semitoneOffset / 12);
        int closest = 0;
        float distance = INFINITY;
        for (int i = 0; i < 12; i++)
        {
            if (validNotes[i] && abs(modOffset - (i - 9)) < distance)
            {
                distance = abs(modOffset - (i-9));
                closest = i - 9;
            }
        }
        float targetFreq = 440.f * pow(TWELTH_ROOT_TWO, roundf(closest + octaveOffset * 12));
        scale *= targetFreq / ffreq;
    }
    scale = 1.f / scale;

    float max = -INFINITY;
    for (int i = bufSize; i < bufSize + (int)period; i++)
    {
        if (inputs[i] > max)
        {
            max = inputs[i];
            marker = i;
        }
    }
    marker -= bufSize;

    // Get all the "markers", i.e. peaks of the input signal
    std::vector<int> markers;
    for (int m = marker; m < bufSize; m+=period) markers.push_back(m);

    // Get windows
    std::vector<std::vector<float>> windows = getWindows(inputs + bufSize, period, markers, scale);

    // Get synthesized markers
    int idx = 0;
    if (markers.size() == 0) return;
    float marker_s = markers[0];

    if (setupFlag && lastMarker < bufSize)
    {
        marker_s = (1.f - ((bufSize - lastMarker) / (prevPeriod * prevFactor))) * period * scale;
    }

    std::vector<int> markers_s;
    for (float m = marker_s; m < bufSize; m += period * scale) {
        markers_s.push_back((int)round(m));
    }

    if (markers_s.empty())
    {
        prevPeriod = period;
        prevFactor = scale;

        memcpy(buf, outputs, sizeof(float) * bufSize);
        return;
    }

    // Overlap add the windows to the output buffer at the synthesized markers
    int closestIdx = 0;
    for (auto marker_s : markers_s)
    {
        // Get closest marker
        float distance = INFINITY;
        for (int i = closestIdx; i < (int)markers.size(); i++)
        {
            if (abs(marker_s - markers[i]) < distance)
            {
                distance = abs(marker_s - markers[i]);
                closestIdx = i;
            } else break;
        }

        addToBuffer(windows[closestIdx], marker_s);
    }

    prevPeriod = period;
    prevFactor = scale;
    lastMarker = markers_s[markers_s.size()-1];


    memcpy(buf, outputs, sizeof(float) * bufSize);
    setupFlag = true;
}

void PitchShift::addToBuffer(std::vector<float> window, int marker)
{
    window = Window::Hann(window);
    int pos;

    for (int i = -window.size() / 2, j = 0; i < (int)window.size()/2; i++, j++) {
        pos = bufSize + marker + i;
        if (pos >= 0 && pos < 3 * bufSize) outputs[pos] += window[j];
    }
}

std::vector<std::vector<float>> PitchShift::getWindows(float *buf, float period, std::vector<int> markers, float scale)
{
    std::vector<std::vector<float>> output = {};
    for (auto marker : markers)
    {
        std::vector<float> window(2*period, 0);
        for (int i = -period; i < period; i++)
        {
            window[i+period] = buf[marker+i];
        }
        if (scale != 1.f) window = resample(window, scale);
        output.push_back(window);
    }
    return output;
}

void PitchShift::add(float *buf)
{
    // Shift buffers by bufSize
    memcpy(inputs, inputs + bufSize, sizeof(float) * bufSize * 2);
    memcpy(outputs, outputs + bufSize, sizeof(float) * bufSize * 2);
    // Add buffer to inputs
    memcpy(inputs + bufSize * 2, buf, sizeof(float) * bufSize);
    // Set end of output buffer to 0s
    memset(outputs + bufSize * 2, 0, sizeof(float) * bufSize);
}


std::vector<float> PitchShift::resample(std::vector<float> input, float scale)
{
    std::vector<float> v(ceil(scale * input.size()), 0.f);
    float i_scaled;
    for (int i = 0; i < v.size(); i++) {
        i_scaled = (float)i / (v.size()-1) * (input.size()-1);
        v[i] = std::lerp(input[floor(i_scaled)],
                         input[ceil(i_scaled)],
                         i_scaled - floor(i_scaled));
    }
    return v;
}

void PitchShift::reset() {
    for (int i = 0; i < bufSize * 3; i++) {
        inputs[i] = 0.f;
        outputs[i] = 0.f;
    }
}
