#ifndef AUTOTUNER_H
#define AUTOTUNER_H

#include "iaudiofx.h"
#include "lib/pitchshift/pitchshifter.h"

class Autotuner : public IAudioFX {
 public:
  Autotuner(std::shared_ptr<PitchShifter> ps);

  void Process(float *buf) override;
  void Reset(int framesPerBuffer = -1) override;
  float Get(std::string item) override;
  void Set(std::string item, int val) override;
  bool GetEnabled() override;
  void SetEnabled(bool enabled) override;

 private:
  std::shared_ptr<PitchShifter> ps;
};

#endif  // AUTOTUNER_H
