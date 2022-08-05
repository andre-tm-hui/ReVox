#ifndef PASSTHROUGH_H
#define PASSTHROUGH_H

#include <QCheckBox>
#include "audiostream.h"
#include "nlohmann/json.hpp"
using namespace nlohmann;

class Passthrough : public AudioStream
{
public:
    Passthrough(device inputDevice,
                device outputDevice,
                int sampleRate,
                int framesPerBuffer,
                std::string dir);

    ~Passthrough()
    {
        delete data.reverb;
        delete data.autotune;
    }

    void Record(int keycode);
    void Stop(int keycode);

    void SetFX(json settings);

    std::map<std::string, QCheckBox*> *checkboxes;

    passthroughData data = {};

    bool recording = false;
};

#endif // PASSTHROUGH_H
