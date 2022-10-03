#ifndef PASSTHROUGH_H
#define PASSTHROUGH_H

#include "audiostream.h"
#include "nlohmann/json.hpp"
#include <hud.h>

using namespace nlohmann;

class Passthrough : public AudioStream
{
public:
    Passthrough(device inputDevice,
                device outputDevice,
                int sampleRate,
                int framesPerBuffer,
                std::string dir,
                float *inputBuffer);

    ~Passthrough()
    {
        delete data.reverb;
        delete data.autotune;
    }

    void Record(int keycode);
    void Stop();

    void SetFX(json settings);

    passthroughData data = {};

    bool recording = false;

    HUD *hud;
};

#endif // PASSTHROUGH_H
