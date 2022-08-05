#ifndef NOISEGENERATOR_H
#define NOISEGENERATOR_H

#include <audiostream.h>

class NoiseGenerator : public AudioStream
{
public:
    NoiseGenerator(device outputDevice, int sampleRate);
};

#endif // NOISEGENERATOR_H
