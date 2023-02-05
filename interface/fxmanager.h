#ifndef FXMANAGER_H
#define FXMANAGER_H

#include "basemanager.h"
#include "../backend/streams/passthrough.h"
#include "../backend/streams/monitor.h"
#include "../ui/hud.h"
#include "../audiofx/autotuner.h"
#include "../audiofx/repitcher.h"
#include "../audiofx/reverberator.h"

class FXManager : public BaseManager
{
public:
    FXManager(std::string rootDir, int framesPerBuffer, int sampleRate);

    void SetHUD(HUD *hud) { this->hud = hud; };
    json GetFXOff();
    json* GetFXOffPointer();

    void SetStreams(std::shared_ptr<Passthrough> passthrough, std::shared_ptr<Monitor> monitor);
    void ResetStreams();
    void SetMonitoringVol(int n) override;

    void ApplyFXSettings(json obj);

    std::unordered_map<std::string, std::shared_ptr<IAudioFX>>* GetFXs() { return &fxs; }

private:
    void StartEvent(std::string path, int idx) override;

    std::shared_ptr<Passthrough> passthrough;
    std::shared_ptr<Monitor> monitor; // or monitor cb_data
    HUD *hud = nullptr;

    std::unordered_map<std::string, std::shared_ptr<IAudioFX>> fxs = {};

    std::shared_ptr<PitchShifter> ps;
};

#endif // FXMANAGER_H
