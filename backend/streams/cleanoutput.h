#ifndef CLEANOUTPUT_H
#define CLEANOUTPUT_H

#include "audiostream.h"

class CleanOutput : public AudioStream {
 public:
  CleanOutput(int inputDevice, int nInputChannels, int outputDevice, int nOutputChannels, int sampleRate,
              int framesPerBuffer);
};

#endif  // CLEANOUTPUT_H
