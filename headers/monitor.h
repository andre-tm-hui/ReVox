#ifndef MONITOR_H
#define MONITOR_H

#include <audiostream.h>

class Monitor : public AudioStream
{
public:
    Monitor(device inputDevice,
            device outputDevice,
            int sampleRate,
            int framesPerBuffer,
            std::string dir,
            float *inputBuffer,
            float *playbackBuffer,
            float *streamBuffer);

    void Record(int keycode);
    void Stop(int keycode);
    void Merge(int keycode);

    monitorData data;
    bool recording = false;
};

#endif // MONITOR_H
