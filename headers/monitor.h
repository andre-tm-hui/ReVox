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
            float *playbackBuffer);

    void Record(int keycode);
    void Stop();
    void Merge();

    monitorData data;

private:
    int keycode;
    bool recording = false;
};

#endif // MONITOR_H
