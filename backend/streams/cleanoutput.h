#ifndef CLEANOUTPUT_H
#define CLEANOUTPUT_H

#include "audiostream.h"

class CleanOutput : public AudioStream {
 public:
  CleanOutput(device inputDevice, device outputDevice, int sampleRate,
              int framesPerBuffer);
};

#endif  // CLEANOUTPUT_H
