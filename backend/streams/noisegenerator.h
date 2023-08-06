#ifndef NOISEGENERATOR_H
#define NOISEGENERATOR_H

#include "audiostream.h"

class NoiseGenerator : public AudioStream {
 public:
  NoiseGenerator(int outputDevice, int nChannels, int sampleRate);
};

#endif  // NOISEGENERATOR_H
