#include "audiofx/autotuner.h"

Autotuner::Autotuner(std::shared_ptr<PitchShifter> ps)
    : IAudioFX("Autotuner"), ps(ps) {
  paramMap["Notes"] = {"FXKeyPicker", "", "Set which notes to tune to", 0, 0};
  paramMap["Speed"] = {"FXDial", "ms", "Set the time to tune a full semitone",
                       0, 500};
}

void Autotuner::Process(float *buf) {
  if ((!ps->getRepitchActive() && ps->getAutotuneActive()) ||
      (ps->getRepitchActive() && ps->getAutotuneActive()))
    ps->process(buf);
}

void Autotuner::Reset(int framesPerBuffer) { ps->reset(); }

float Autotuner::Get(std::string item) {
  if (item == "Notes") {
    std::vector<bool> notes = ps->getAutotuneNotes();
    int bitmask = 0, inc = 1;
    for (bool note : notes) {
      if (note) bitmask += inc;
      inc *= 2;
    }
    return bitmask;  // will need to be rounded when received on the other end
  } else if (item == "Speed") {
    return ps->getRetuneSpeed();
  }
  return 0.f;
}

void Autotuner::Set(std::string item, int val) {
  log(INFO, "Setting " + item + " to " + std::to_string(val));
  if (item == "Notes") {
    std::vector<bool> notes(12, false);
    int inc = pow(2, 11), bitmask = val;
    for (int i = 11; i >= 0; i--) {
      if (bitmask >= pow(2, i)) {
        bitmask -= inc;
        notes[i] = true;
      }
      inc /= 2;
    }
    ps->setAutotuneNotes(notes);
  } else if (item == "Speed") {
    ps->setRetuneSpeed(val);
  }
}

bool Autotuner::GetEnabled() { return ps->getAutotuneActive(); }

void Autotuner::SetEnabled(bool enabled) {
  log(INFO, enabled ? "Enabling autotuner" : "Disabling autotuner");
  ps->setAutotuneActive(enabled);
}
