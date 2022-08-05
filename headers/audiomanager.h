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
    void SetCheckboxes(std::map<std::string, QCheckBox*> *checkboxes);
    void OverrideConfig(std::string keycode);
    void OverrideSound(std::string fname, int keycode);

    void SetNumberOfSounds(int n) { settings["maxNumberOfSounds"] = n; player->maxLiveSamples = n; }
    void SetPlaybackLength(int n) { settings["maxFileLength"] = n; player->data.maxFileLength = n; }

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
            "inputDevice": 0,
            "outputDevice": -1,
            "streamOutputDevice": 0,
            "virtualInputDevice": 0,
            "virtualOutputDevice": 0,
            "sampleRate": 48000,
            "framesPerBuffer": 2048,
            "maxNumberOfSounds": 3,
            "maxFileLength": 5
        }
        )"_json;

private:
    std::map<std::string, QCheckBox*> *checkboxes;
    int sampleRate, framesPerBuffer, defVInput, defVOutput;
    int rebindAt = -1;

    std::string appdata;
    std::string dirName = "/Virtual SoundTool/";

    void SaveSettings();
    void SetupStreams();

    device GetDeviceByIndex(int i);
    void GetDeviceSettings();
    int GetCorrespondingLoopbackDevice(int i);

    json baseSoundboardHotkey = R"(
        {
            "label": "",
            "recordInput": true,
            "recordLoopback": true,
            "syncStreams": true
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
