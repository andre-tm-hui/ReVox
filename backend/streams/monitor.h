#ifndef MONITOR_H
#define MONITOR_H

#include "audiostream.h"

class Monitor : public AudioStream {
 public:
  Monitor(int inputDevice, int nInputChannels, int outputDevice, int nOutputChannels, int sampleRate,
          int framesPerBuffer, std::string dir, std::queue<float> *inputQueue,
          std::queue<float> *playbackQueue);

  void SetFXMonitorVol(float val) { data.monitorMic = val; }
  void SetSoundboardMonitorVol(float val) { data.monitorSamples = val; }

 private:
  monitorData data;
};

#endif  // MONITOR_H
