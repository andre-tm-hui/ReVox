#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "passthrough.h"
#include "player.h"
#include "monitor.h"
#include "cleanoutput.h"
#include "noisegenerator.h"
#include <QCheckBox>
#include <nlohmann/json.hpp>
#include <samplerate.h>
using namespace nlohmann;

// struct containing all settings related to a keybind
typedef struct{
    int type;
    bool recordInput;
    bool recordLoopback;
    bool padAudio;

    int fxType;

    float roomsize;
    float damp;
    float width;
    float wet;
    float dry;
} keybind;

typedef struct{
    int input;
    int output;
    int streamOut;
    int vInput;
    int vOutput;
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

    void Reset(int input, int output, int stream);

    void Rebind(int keycode);
    void SetNewBind(int keycode, bool isSoundboard);
    void RemoveBind(int keycode);
    void SaveBinds();
    void SaveSettings();
    void SetCheckboxes(std::map<std::string, QCheckBox*> *checkboxes);
    void OverrideConfig(std::string keycode);
    void OverrideSound(std::string fname, int keycode);

    void SetNumberOfSounds(int n) { settings["maxNumberOfSounds"] = n; player->maxLiveSamples = n; }
    void SetPlaybackLength(int n) { settings["maxFileLength"] = n; player->data.maxFileLength = n; }
    void SetSampleMonitor(int n) { monitor->data.monitorSamples = (float)n / 99.f; settings["monitorSamples"] = n; SaveSettings(); }
    void SetMicMonitor(int n) { monitor->data.monitorMic = (float)n / 99.f; settings["monitorMic"] = n; SaveSettings(); }

    void WaitForReady();

    Passthrough *passthrough;
    Player *player;
    Monitor *monitor;
    CleanOutput *cleanOutput;
    NoiseGenerator *noiseGen;
    json soundboardHotkeys;
    json voiceFXHotkeys;

    bool recording = false;

    json settings = R"(
        {
            "inputDevice": "",
            "outputDevice": "",
            "streamOutputDevice": "",
            "virtualInputDevice": "",
            "virtualOutputDevice": "",
            "sampleRate": 48000,
            "framesPerBuffer": 2048,
            "maxNumberOfSounds": 3,
            "maxFileLength": 5,
            "monitorSamples": 0,
            "monitorMic": 0
        }
        )"_json;

private:
    std::map<std::string, QCheckBox*> *checkboxes;
    int sampleRate, framesPerBuffer, defVInput, defVOutput;
    deviceIDs ids = {-1, -1, -1, -1, -1};
    int rebindAt = -1;

    std::string appdata;
    std::string dirName = "/Virtual SoundTool/";


    void SetupStreams();
    static void ShutdownStreams(Passthrough *passthrough,
                                Player *player,
                                Monitor *monitor,
                                CleanOutput *cleanOutput,
                                NoiseGenerator *noiseGen);
    static void CheckForDeviceChanges(Passthrough *passthrough,
                                      Player *player,
                                      Monitor *monitor,
                                      CleanOutput *cleanOutput,
                                      NoiseGenerator *noiseGen);
    device GetDeviceByIndex(int i);
    void GetDeviceSettings();
    int GetCorrespondingLoopbackDevice(int i);

    json baseSoundboardHotkey = R"(
        {
            "label": "",
            "recordInput": true,
            "recordLoopback": true
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
};

#endif // AUDIOMANAGER_H
