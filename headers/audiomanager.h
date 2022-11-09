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
#define VER_NO "0.1"
#endif

using namespace nlohmann;

enum bindType {
    invalid = -1,
    soundboard,
    voicefx,
    recordOver
};

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

    void KeyEvent(int keycode, std::string deviceName, int event);

    void Record(std::string idx);
    void StopRecording();
    void Play(std::string idx, bool recordFallback = true);
    void OverrideSound(std::string fname, int idx);

    void Reset(bool devicesChanged = false);

    void StartRebind();
    bool IsRebinding() { return rebinding; };
    int GetRebindDestination() { return rebindTo; }
    std::string GetRebindDevice() { return rebindDevice; }
    void SetNewBind(int keycode, std::string deviceName, int idx, bindType bindType);
    void RemoveBind(int idx, bindType bindType);

    void SaveBinds();
    void SaveSettings();

    void SetHUD(HUD *hud);

    void SetWaveform(WaveformViewer *wv) { this->wv = wv; }
    void ResetWaveform() { this->wv->SetAudioClip(); }

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

    json settings;

    int *fxHotkey;

private:
    int sampleRate, framesPerBuffer, defVInput, defVOutput;
    deviceIDs ids = {-1, -1, -1, -1};
    int rebindAt = -1, rebindTo = -1;
    std::string rebindDevice = "";
    bool rebinding = false, recordOver = false;

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

    std::pair<bindType, std::string> isExistingKeybind(int keycode, std::string deviceName);

    json defaultSettings = R"(
        {
            "outputDevice": "No Device Detected",
            "sampleRate": 48000,
            "framesPerBuffer": 2048,
            "hudPosition": -1,
            "startWithWindows": false,
            "autocheckUpdates": false,
            "firstTime": true,
            "monitorMic": 1.0,
            "monitorSamples": 1.0,
            "recordOverKeybind": -1,
            "recordOverDeviceName": ""
        }
        )"_json;

    json baseSoundboardHotkey = R"(
        {
            "keycode": -1,
            "deviceName": "",
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
            "keycode": -1,
            "deviceName": "",
            "label": "",
            "reverb": { "enabled": false, "roomsize": 0.5, "mix": 0.5, "width": 0.5, "damp": 0.5 },
            "autotune": { "enabled": false, "speed": 0.5, "notes": [ true, false, true, false, true, true, false, true, false, true, false, true ] },
            "pitch": { "enabled": false, "pitch": 1.0 },
            "editable": true
        }
        )"_json;

    float *inputBuffer, *playbackBuffer;

    HUD *hud;
    WaveformViewer *wv;
};

#endif // AUDIOMANAGER_H
