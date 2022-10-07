#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "passthrough.h"
#include "player.h"
#include "monitor.h"
#include "cleanoutput.h"
#include "noisegenerator.h"
#include <QWidget>
#include <nlohmann/json.hpp>
#include <samplerate.h>
#include <waveformviewer.h>

#ifndef VER_NO
#define VER_NO "1.0.0"
#endif

using namespace nlohmann;

typedef struct{
    int input;
    int output;
    int streamOut;
    int vInput;
} deviceIDs;

class AudioManager
{
public:
    AudioManager();
    ~AudioManager();

    std::map<std::string, int> deviceList;
    std::map<std::string, device> inputDevices;
    std::map<std::string, device> outputDevices;
    std::map<std::string, device> loopbackDevices;

    void Record(int keycode);
    void StopRecording();
    void Play(int keycode, bool recordFallback = true);

    void Reset(bool devicesChanged = false);

    void Rebind(int keycode);
    void SetNewBind(int keycode, bool isSoundboard);
    void RemoveBind(int keycode);
    void SaveBinds();
    void SaveSettings();
    void SetHUD(HUD *hud);
    void SetWaveform(WaveformViewer *wv) { this->wv = wv; }
    void ResetWaveform() { this->wv->SetAudioClip(); }
    void OverrideSound(std::string fname, int keycode);

    void SetSampleMonitor(int n) { monitor->data.monitorSamples = (float)n / 100.f; settings["monitorSamples"] = n; SaveSettings(); }
    void SetMicMonitor(int n) { monitor->data.monitorMic = (float)n / 100.f; settings["monitorMic"] = n; SaveSettings(); }

    int GetCurrentOutputDevice() { return ids.output; }
    void SetCurrentOutputDevice(int id) { ids.output = id; settings["outputDevice"] = Pa_GetDeviceInfo(ids.output)->name; Reset(); }
    json GetFXOff() { return baseFXHotkey; }
    json* GetFXOffPointer() { return &baseFXHotkey; }

    std::string GetAudioPath(int keycode)
    {
        return appdata + dirName + "samples/" + std::to_string(keycode) + ".mp3";
    }

    void WaitForReady();

    Passthrough *passthrough;
    Player *player;
    Monitor *monitor;
    CleanOutput *cleanOutput;
    NoiseGenerator *noiseGen;
    json soundboardHotkeys;
    json voiceFXHotkeys;
    QWidget *fxMenu;

    bool recording = false;

    json settings = R"(
        {
            "outputDevice": "",
            "sampleRate": 48000,
            "framesPerBuffer": 2048,
            "hudPosition": 2,
            "monitorMic": 0,
            "monitorSamples": 0,
            "startWithWindows": false,
            "autocheckUpdates": false
        }
        )"_json;

    int *fxHotkey;

private:
    //std::map<std::string, QCheckBox*> *checkboxes;
    int sampleRate, framesPerBuffer, defVInput, defVOutput;
    deviceIDs ids = {-1, -1, -1, -1};
    int rebindAt = -1;

    std::string appdata;
    std::string dirName = "/ReVox/";


    void SetupStreams();
    static void ShutdownStreams(Passthrough *passthrough,
                                Player *player,
                                Monitor *monitor,
                                CleanOutput *cleanOutput,
                                NoiseGenerator *noiseGen);

    device GetDeviceByIndex(int i);
    void GetDeviceSettings();
    int GetChannels(int id, bool isInput);
    int GetCorrespondingLoopbackDevice(int i);

    json baseSoundboardHotkey = R"(
        {
            "label": "",
            "recordInput": true,
            "recordLoopback": true,
            "startAt": 0,
            "endAt": -1,
            "maxCopies": 1,
            "volume": 1.0
        }
        )"_json;

    json baseFXHotkey = R"(
        {
            "label": "",
            "reverb": { "enabled": false, "roomsize": 0.5, "mix": 0.5, "width": 0.5, "damp": 0.5 },
            "autotune": { "enabled": false, "speed": 0.5, "notes": [ true, false, true, false, true, true, false, true, false, true, false, true ] },
            "pitch": { "enabled": false, "pitch": 1.0 }
        }
        )"_json;

    float *inputBuffer, *playbackBuffer;

    HUD *hud;
    WaveformViewer *wv;
};

#endif // AUDIOMANAGER_H
