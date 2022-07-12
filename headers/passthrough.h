#ifndef PASSTHROUGH_H
#define PASSTHROUGH_H

#include "audiostream.h"

class Passthrough : public AudioStream
{
public:
    Passthrough(device inputDevice,
                device outputDevice,
                int sampleRate,
                int framesPerBuffer);
};

#endif // PASSTHROUGH_H
