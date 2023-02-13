#ifndef FXMANAGER_H
#define FXMANAGER_H

#include "../audiofx/autotuner.h"
#include "../audiofx/repitcher.h"
#include "../audiofx/reverberator.h"
#include "../backend/streams/monitor.h"
#include "../backend/streams/passthrough.h"
#include "../ui/hud.h"
#include "basemanager.h"

class FXManager : public BaseManager {
 public:
  FXManager(std::string rootDir, int framesPerBuffer, int sampleRate);

  void SetHUD(HUD* hud) { this->hud = hud; };
  json GetFXOff() { return defaultObj; }
  json* GetFXOffPointer() { return &defaultObj; }

  void SetStreams(std::shared_ptr<Passthrough> passthrough,
                  std::shared_ptr<Monitor> monitor);
  void ResetStreams();
  void SetMonitoringVol(int n) override;

  void ApplyFXSettings(json obj);
  void SetFramesPerBuffer(int framesPerBuffer);

  std::unordered_map<std::string, std::shared_ptr<IAudioFX>>* GetFXs() {
    return &fxs;
  }

 private:
  void StartEvent(std::string path, int idx) override;

  std::shared_ptr<Passthrough> passthrough;
  std::shared_ptr<Monitor> monitor;  // or monitor cb_data
  HUD* hud = nullptr;

  std::unordered_map<std::string, std::shared_ptr<IAudioFX>> fxs = {};

  std::shared_ptr<PitchShifter> ps;
  int sampleRate, framesPerBuffer;
};

#endif  // FXMANAGER_H
