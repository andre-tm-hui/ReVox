#ifndef SOUNDBOARDMANAGER_H
#define SOUNDBOARDMANAGER_H

#include <chrono>

#include "../backend/streams/monitor.h"
#include "../backend/streams/passthrough.h"
#include "../backend/streams/player.h"
#include "../ui/widgets/soundboard/waveformviewer.h"
#include "../util/loggableobject.h"
#include "basemanager.h"

class SoundboardManager : public BaseManager {
 public:
  SoundboardManager(std::string rootDir, int sampleRate);

  void Record(std::string idx);
  void StopRecording();
  void Play(std::string idx, int ev = 0);
  void StopAll();
  void Stop(std::string idx);
  void OverrideSound(std::string fname, int idx);
  void SetWaveform(WaveformViewer *wv);

  void SetStreams(std::shared_ptr<Passthrough> passthrough,
                  std::shared_ptr<Monitor> monitor,
                  std::shared_ptr<Player> player);
  void ResetStreams();
  void SetMonitoringVol(int n) override;
  void SetPadding(int ms);

 private:
  void StartEvent(std::string path, int event) override;
  void StopRecordingAfter();

  std::shared_ptr<Passthrough> passthrough;
  std::shared_ptr<Monitor> monitor;
  std::shared_ptr<Player> player;
  WaveformViewer *wv = nullptr;
  bool recording = false, stopping = false, modifier = false;
  int sampleRate = 0, holding = 0;
  std::string currHotkey = "";

  std::shared_ptr<spdlog::logger> logger;
};

#endif  // SOUNDBOARDMANAGER_H
