#ifndef PASSTHROUGH_H
#define PASSTHROUGH_H

#include "audiostream.h"
#include "../../ui/hud.h"

class Passthrough : public AudioStream
{
public:
    Passthrough(device inputDevice,
                device outputDevice,
                int sampleRate,
                int framesPerBuffer,
                std::string dir,
                std::queue<float> *bufQueue);

    ~Passthrough()
    {
        delete data.rData;
    }

    void Record(int keycode);
    void Stop();

    void SetPadding(int padding);
    void SetFXList(std::unordered_map<std::string, std::shared_ptr<IAudioFX>> *fxs) { this->data.fxs = fxs; }

    passthroughData data = {};

    bool recording = false;

    HUD *hud;

private:
    RingBuffer<float> pBuf;
    int padding = 0;
};

#endif // PASSTHROUGH_H
