#include "fxmanager.h"

FXManager::FXManager(std::string rootDir, int framesPerBuffer, int sampleRate) : BaseManager(rootDir)
{
    ps.reset(new PitchShifter(2048, framesPerBuffer, sampleRate));
    /*SF_INFO info = {};
    info.format = 0;
    SNDFILE *inputFile = sf_open((rootDir + "/samples/3.mp3").c_str(), SFM_READ, &info);
    SF_INFO outInfo = {};
    outInfo.channels = 1;
    outInfo.samplerate = info.samplerate;
    outInfo.format = info.format;
    SNDFILE *outputFile = sf_open((rootDir + "/samples/3-autotuned.mp3").c_str(), SFM_WRITE, &outInfo);
    sf_count_t count = 0;
    float *in = new float[4096], *out = new float[2048];
    int total = 0;
    while (true) {
        count = sf_read_float(inputFile, in, 4096);
        for (int i = 0; i < 2048; i++) {
            out[i] = in[2*i] + in[2*i+1];
        }
        ps->process(out);
        //std::cout<<count<<std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        if (total >= 2048 * 3) {
            sf_write_float(outputFile, out, count / 2);
        } else {
            total+=2048;
        }
        //std::cout<<count<<std::endl;
        if (count < 4096) break;
    }
    sf_close(inputFile);
    sf_close(outputFile);*/
    fxs.insert({"Repitcher", std::unique_ptr<IAudioFX>(new Repitcher(ps))});
    fxs.insert({"Autotuner", std::unique_ptr<IAudioFX>(new Autotuner(ps))});
    fxs.insert({"Reverberator", std::unique_ptr<IAudioFX>(new Reverberator(framesPerBuffer))});

    defaultObj = R"({
        "keycode": -1,
        "deviceName": "",
        "label": "",
        "editable": true,
        "Reverberator": {
            "enabled": false,
            "Roomsize": 50,
            "Wet": 50,
            "Dry": 50,
            "Damp": 50,
            "Width": 50
        },
        "Autotuner": {
            "enabled": false,
            "Speed": 50,
            "Notes": 2741
        },
        "Repitcher": {
            "enabled": false,
            "Pitch": 0
        }
    })"_json;


    dataPath = "voicefxSettings.json";
    settings = R"({
        "monitor": 100,
        "hotkeys": {}
    })"_json;

    LoadSettings();

    if (settings["hotkeys"].size() == 0) {
        settings["hotkeys"]["0"] = defaultObj;
        settings["hotkeys"]["0"]["editable"] = false;
        settings["hotkeys"]["0"]["label"] = "Off";
        SaveSettings();
    }


}

void FXManager::StartEvent(std::string path, int event) {
    if (path.find("hotkeys") != path.npos) {
        std::string idx = path.substr(path.find("/") + 1);
        json bindSettings = settings["hotkeys"][idx];
        if (bindSettings.empty()) return;
        if (event == 0 && passthrough != nullptr) {
            ApplyFXSettings(settings["hotkeys"][idx]);
        }
    } else {
        // handle all other hotkey events
    }
}

json FXManager::GetFXOff() {
    return defaultObj;
}

json* FXManager::GetFXOffPointer() {
    return &defaultObj;
}

void FXManager::SetStreams(std::shared_ptr<Passthrough> passthrough, std::shared_ptr<Monitor> monitor) {
    this->passthrough = passthrough;
    passthrough->SetFXList(&fxs);
    this->monitor = monitor;
    monitor->SetFXMonitorVol(settings["monitor"].get<int>() / 100.f);
    ApplyFXSettings(defaultObj);
}

void FXManager::ResetStreams() {
    this->passthrough.reset();
    this->monitor.reset();
}

void FXManager::SetMonitoringVol(int n) {
    UpdateSettings<int>("monitor", n);
    monitor->SetFXMonitorVol(settings["monitor"].get<int>() / 100.f);
}

void FXManager::ApplyFXSettings(json obj) {
    for (auto &[fx, newSettings] : obj.items()) {
        if (fxs.find(fx) != fxs.end()) {
            for (auto &[setting, val] : newSettings.items()) {
                if (setting == "enabled") {
                    if (!fxs[fx]->GetEnabled()) {
                        fxs[fx]->Reset();
                    }
                    if (hud != nullptr) {
                        if (val)
                            hud->AddActiveEffect(QString::fromStdString(fx));
                        else
                            hud->RemoveActiveEffect(QString::fromStdString(fx));
                    }
                    fxs[fx]->SetEnabled(val.get<bool>());
                } else {
                    fxs[fx]->Set(setting, val.get<int>());
                }
            }
        } else
            return;
    }
}
