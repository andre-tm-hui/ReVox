#ifndef SOUNDBOARDMANAGER_H
#define SOUNDBOARDMANAGER_H

#include "basemanager.h"
#include "../ui/widgets/soundboard/waveformviewer.h"
#include "../backend/streams/passthrough.h"
#include "../backend/streams/monitor.h"
#include "../backend/streams/player.h"

class SoundboardManager : public BaseManager
{
public:
    SoundboardManager(std::string rootDir, int sampleRate);

    void Record(std::string idx);
    void StopRecording();
    void Play(std::string idx);
    void StopAll();
    void OverrideSound(std::string fname, int idx);
    void SetWaveform(WaveformViewer *wv);

    void SetStreams(std::shared_ptr<Passthrough> passthrough,
                    std::shared_ptr<Monitor> monitor,
                    std::shared_ptr<Player> player);
    void ResetStreams();
    void SetMonitoringVol(int n) override;

private:
    void StartEvent(std::string path, int event) override;

    std::shared_ptr<Passthrough> passthrough;
    std::shared_ptr<Monitor> monitor;
    std::shared_ptr<Player> player;
    WaveformViewer *wv = nullptr;
    bool recording = false,
         recordOver = false;
    int sampleRate = 0;
    std::string currHotkey = "";
};

#endif // SOUNDBOARDMANAGER_H
