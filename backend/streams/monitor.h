#ifndef MONITOR_H
#define MONITOR_H

#include "audiostream.h"

class Monitor : public AudioStream
{
public:
    Monitor(device inputDevice,
            device outputDevice,
            int sampleRate,
            int framesPerBuffer,
            std::string dir,
            float *inputBuffer,
            float *playbackBuffer);

    void Record(int keycode);
    void Stop();
    void Merge();
    void SetFXMonitorVol(float val) { data.monitorMic = val; }
    void SetSoundboardMonitorVol(float val) { data.monitorSamples = val; }
    void SetPadding(int padding);

private:
    RingBuffer<float> pBuf;
    int keycode,
        padding = 0;
    bool recording = false;

    monitorData data;
};

#endif // MONITOR_H
