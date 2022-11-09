#include "audiomanager.h"

AudioManager::AudioManager()
{
    // initialize portaudio for the entire app
    Pa_Initialize();

    // get the %appdata% location - all Windows machines should pass the appdata_c check
    char* appdata_c = std::getenv("APPDATA");
    if (appdata_c)
    {
        appdata = appdata_c;
        // if a folder for the app does not exist, create the folder and it's subdirectories, and initialize some default configuration files
        if (!std::filesystem::exists(appdata + dirName))
        {
            // create a storage directory
            std::filesystem::create_directory(appdata + dirName);
            std::filesystem::create_directory(appdata + dirName + "samples/");

            settings = defaultSettings;
            SaveSettings();
        }
        // otherwise, load saved configurations
        else
        {
            std::ifstream settingsFile (appdata + dirName + "settings.json");
            if (!settingsFile.fail())
            {
                settingsFile >> settings;
                for (auto const& [k,v] : defaultSettings.items())
                    if (settings.find(k) == settings.end())
                        settings[k] = v;
            }

            std::ifstream soundboardBindsFile(appdata + dirName + "soundboard.json");
            if (!soundboardBindsFile.fail())
            {
                soundboardBindsFile >> soundboardHotkeys;
                for (auto& [idx,keybind] : soundboardHotkeys.items())
                    for (auto const& [k,v] : baseSoundboardHotkey.items())
                        if (keybind.find(k) == keybind.end())
                            keybind[k] = v;
            }

            std::ifstream voiceFXBindsFile(appdata + dirName + "voicefx.json");
            if (!voiceFXBindsFile.fail())
            {
                voiceFXBindsFile >> voiceFXHotkeys;
                for (auto& [idx,keybind] : voiceFXHotkeys.items())
                    for (auto const& [k,v] : baseFXHotkey.items()) {
                        if (keybind.find(k) == keybind.end())
                            keybind[k] = v;
                        else if (v.is_object())
                            for (auto const& [k1,v1] : v.items())
                               if (keybind[k].find(k1) == keybind[k].end())
                                   keybind[k][k1] = v1;
                    }
            }
        }
    }

    GetDeviceSettings();

    // start the portaudio stream objects
    SetupStreams();
}

AudioManager::~AudioManager()
{
    SaveSettings();
    SaveBinds();
}

void AudioManager::KeyEvent(int keycode, std::string deviceName, int event)
{
    std::pair<bindType, std::string> bind = isExistingKeybind(keycode, deviceName);
    bindType bt = bind.first;
    std::string idx = bind.second;

    if (rebinding) {
        if ((bt == bindType::soundboard || bt == bindType::voicefx) && idx.size() > 0) return;
        rebindDevice = deviceName;
        switch (keycode) {
        case 8: // bksp to clear
            rebindTo = -2;
            break;
        case 27: // esc to cancel
            rebindTo = -1;
            break;
        default:
            rebindTo = keycode;
        }

        rebinding = false;
    } else {
        json bindSettings;
        switch (bt) {
        case bindType::invalid:
            return;
        case bindType::soundboard:
            if (idx.size() == 0) return;
            bindSettings = soundboardHotkeys[idx];
            if (bindSettings.empty()) return;
            if (event == 0)
                Play(idx);
            else
                if (recording)
                    StopRecording();
            break;
        case bindType::voicefx:
            if (idx.size() == 0) return;
            bindSettings = voiceFXHotkeys[idx];
            if (bindSettings.empty()) return;
            if (event == 0 && passthrough != nullptr) {
                passthrough->SetFX(bindSettings);
                *fxHotkey = keycode;
            }
            break;
        case bindType::recordOver:
            if (event == 0)
                this->recordOver = true;
            else
                this->recordOver = false;
            break;
        }
    }
}

void AudioManager::Record(std::string idx)
{
    json keyData = soundboardHotkeys[idx];
    if ((keyData["recordInput"] ||
         keyData["recordLoopback"]) &&
            std::filesystem::exists(appdata + dirName + "samples/" + idx + ".mp3"))
        std::filesystem::remove(appdata + dirName + "samples/" + idx + ".mp3");

    if (keyData["recordInput"] && passthrough != nullptr) passthrough->Record(std::stoi(idx));
    if (keyData["recordLoopback"] && monitor != nullptr) monitor->Record(std::stoi(idx));

    recording = true;
}

void AudioManager::StopRecording()
{
    if (passthrough != nullptr) passthrough->Stop();
    if (monitor != nullptr) monitor->Stop();

    recording = false;
    wv->SetAudioClip();
}

void AudioManager::Play(std::string idx, bool recordFallback)
{
    if (player != nullptr && !recording)
    {
        bool playable = player->CanPlay(std::stoi(idx));
        if (this->recordOver || !playable)
            Record(idx);
        else if ( playable) {
            player->Play(std::stoi(idx), soundboardHotkeys[idx]);
        }
    }
}

void AudioManager::WaitForReady()
{
    while (!monitor->initialSetup ||
           !player->initialSetup ||
           !passthrough->initialSetup ||
           !noiseGen->initialSetup) {}

    if (cleanOutput != nullptr) while (!cleanOutput->initialSetup) {}
}

void AudioManager::GetDeviceSettings()
{
    std::map<std::string, int> apiMap;
    ids = {-1, -1, -1, -1};
    deviceList = {};
    inputDevices = {};
    outputDevices = {};
    loopbackDevices = {};

    // get all available audio APIs on the system
    for (int i = 0; i < Pa_GetHostApiCount(); i++)
    {
        apiMap[Pa_GetHostApiInfo(i)->name] = i;
    }

    // if WASAPI is available, we can check for loopback devices. WASAPI should be available on all modern Win10+ systems.
    if (apiMap.find("Windows WASAPI") != apiMap.end())
    {
        // iterate through all WASAPI devices
        for (int i = 0; i < Pa_GetHostApiInfo(apiMap["Windows WASAPI"])->deviceCount; i++)
        {
            // separate normal devices from loopback devices
            std::string deviceName = Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(apiMap["Windows WASAPI"], i))->name;
            if (deviceName.find("[Loopback]") != std::string::npos)
            {
                loopbackDevices[deviceName] = {Pa_HostApiDeviceIndexToDeviceIndex(apiMap["Windows WASAPI"], i), 2};
                std::string outputDeviceName = deviceName.substr(0, deviceName.size()-11);
                outputDevices[outputDeviceName] = {deviceList[outputDeviceName], GetChannels(deviceList[outputDeviceName], false)};
                if (outputDeviceName.find(settings["outputDevice"].get<std::string>()) != std::string::npos)
                {
                    ids.output = outputDevices[outputDeviceName].id;
                    settings["outputDevice"] = outputDeviceName;
                }
            }
            else
            {
                // add the device to a list of devices if it's not a loopback
                deviceList[deviceName] = Pa_HostApiDeviceIndexToDeviceIndex(apiMap["Windows WASAPI"], i);

                // set the default virtual input device - assumes the user is using VB-Audio's Virtual Cable https://vb-audio.com/Cable/
                if (deviceName.find("CABLE Input") != std::string::npos)
                {
                    ids.vInput = deviceList[deviceName];
                }
                else if (deviceName.find(Pa_GetDeviceInfo(Pa_GetDefaultInputDevice())->name) != std::string::npos)
                {
                    ids.input = deviceList[deviceName];
                }
                else if (deviceName.find(Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->name) != std::string::npos)
                {
                    ids.streamOut = deviceList[deviceName];
                }
            }
        }
    }
    else
    {
        //end execution
    }

    for (auto const& [name, id] : deviceList)
    {
        if (outputDevices.find(name) == outputDevices.end() && loopbackDevices.find(name) == loopbackDevices.find(name))
        {
            inputDevices[name] = {id, GetChannels(id, true)};
        }
    }

    if (ids.input == -1) ids.input = Pa_GetDefaultInputDevice();
    if (ids.streamOut == -1) ids.streamOut = Pa_GetDefaultOutputDevice();
    if (ids.output == -1) ids.output = Pa_GetDefaultOutputDevice();
}

int AudioManager::GetChannels(int id, bool isInput)
{
    PaStreamParameters params;
    params.hostApiSpecificStreamInfo = NULL;
    params.sampleFormat = PA_SAMPLE_TYPE;
    params.device = id;
    params.hostApiSpecificStreamInfo = NULL;
    params.sampleFormat = PA_SAMPLE_TYPE;
    params.channelCount = 2;
    params.suggestedLatency = Pa_GetDeviceInfo(id)->defaultLowInputLatency;

    PaError err = Pa_IsFormatSupported(isInput ? &params : NULL, isInput ? NULL : &params, 48000);
    return err != 0 ? 1 : 2;
}

int AudioManager::GetCorrespondingLoopbackDevice(int i)
{
    std::string dName = Pa_GetDeviceInfo(i)->name;
    if (dName.find("[Loopback]") != std::string::npos) return i;
    for (auto const& [name, id] : loopbackDevices)
    {
        if (name.find(dName) != std::string::npos) return id.id;
    }
    return -1;
}

/* Reset functions for portaudio objects, called when corresponding devices are changed in the GUI */
void AudioManager::Reset(bool devicesChanged)
{
    if (passthrough != nullptr) delete passthrough;
    if (player != nullptr) delete player;
    if (monitor != nullptr) delete monitor;
    if (cleanOutput != nullptr) delete cleanOutput;
    if (noiseGen != nullptr) delete noiseGen;

    Pa_Terminate();
    Pa_Initialize();

    GetDeviceSettings();

    SetupStreams();

    WaitForReady();

    if (!devicesChanged) { SaveSettings(); }
}

void AudioManager::SetupStreams()
{
    inputBuffer = new float[settings["framesPerBuffer"].get<int>() * 2 * 3];
    playbackBuffer = new float[settings["framesPerBuffer"].get<int>() * 2 * 3];

    int loopbackdevice = GetCorrespondingLoopbackDevice(ids.streamOut);
    noiseGen = new NoiseGenerator(GetDeviceByIndex(ids.streamOut), settings["sampleRate"].get<int>());
    monitor             = new Monitor(      GetDeviceByIndex(loopbackdevice),
                                            GetDeviceByIndex(ids.output),
                                            settings["sampleRate"].get<int>(),
                                            settings["framesPerBuffer"].get<int>(),
                                            appdata + dirName,
                                            inputBuffer, playbackBuffer);
    player              = new Player(       GetDeviceByIndex(ids.vInput),
                                            settings["sampleRate"].get<int>(),
                                            settings["framesPerBuffer"].get<int>(),
                                            appdata + dirName,
                                            playbackBuffer);
    passthrough         = new Passthrough(  GetDeviceByIndex(ids.input),
                                            GetDeviceByIndex(ids.vInput),
                                            settings["sampleRate"].get<int>(),
                                            settings["framesPerBuffer"].get<int>(),
                                            appdata + dirName,
                                            inputBuffer);

    if (ids.streamOut != ids.output)
        cleanOutput     = new CleanOutput(  GetDeviceByIndex(loopbackdevice),
                                            GetDeviceByIndex(ids.output),
                                            settings["sampleRate"].get<int>(),
                                            1);
    else
        cleanOutput = nullptr;

    SetSampleMonitor(settings["monitorSamples"].get<int>());
    SetMicMonitor(settings["monitorMic"].get<int>());

    SetHUD(this->hud);
}

void AudioManager::ShutdownStreams(Passthrough *passthrough,
                                   Player *player,
                                   Monitor *monitor,
                                   CleanOutput *cleanOutput,
                                   NoiseGenerator *noiseGen)
{
    delete noiseGen;
    delete monitor;
    delete player;
    delete passthrough;
    delete cleanOutput;
}

/* Configuration save functions */
void AudioManager::SaveSettings()
{
    std::ofstream settingsFile(appdata + dirName + "settings.json");
    settingsFile << std::setw(4) << settings << std::endl;
}

void AudioManager::SetHUD(HUD *hud)
{
    this->hud = hud;
    passthrough->hud = hud;
}

void AudioManager::SaveBinds()
{
    std::ofstream soundboardBindsFile(appdata + dirName + "soundboard.json");
    soundboardBindsFile << std::setw(4) << soundboardHotkeys << std::endl;

    std::ofstream voiceFXBindsFile(appdata + dirName + "voicefx.json");
    voiceFXBindsFile << std::setw(4) << voiceFXHotkeys << std::endl;
}

/* Functions handling hotkey binding/rebinding */
void AudioManager::StartRebind()
{
    rebinding = true;
}

void AudioManager::SetNewBind(int keycode, std::string deviceName, int idx, bindType bindType)
{
    json *obj, entry;
    switch (bindType) {
    case bindType::soundboard: case bindType::voicefx:
        if (bindType == soundboard)
        {
            obj = &soundboardHotkeys;
            entry = baseSoundboardHotkey;
        }
        else
        {
            obj = &voiceFXHotkeys;
            entry = baseFXHotkey;
        }

        entry["keycode"] = keycode;
        entry["deviceName"] = deviceName;
        if (obj->find(std::to_string(idx)) == obj->end())
            (*obj)[std::to_string(idx)] = entry;
        else {
            obj = &((*obj)[std::to_string(idx)]);
            (*obj)["keycode"] = keycode;
            (*obj)["deviceName"] = deviceName;
        }
        SaveBinds();
        break;
    case bindType::recordOver:
        settings["recordOverKeybind"] = keycode;
        if (keycode >= 0)
            settings["recordOverDeviceName"] = deviceName;
        else
            settings["recordOverDeviceName"] = "";
        SaveSettings();
        break;
    }
}

void AudioManager::RemoveBind(int idx, bindType bindType)
{
    json *obj = bindType == soundboard ? &soundboardHotkeys : &voiceFXHotkeys;
    if (bindType == soundboard && std::filesystem::exists(appdata + dirName + "samples/" + std::to_string(idx) + ".mp3"))
        std::filesystem::remove(appdata + dirName + "samples/" + std::to_string(idx) + ".mp3");
    int i;
    for (i = 0; i < obj->size(); i++) {
        if (i > idx) {
            (*obj)[std::to_string(i - 1)] = (*obj)[std::to_string(i)];
            if (bindType == soundboard && player->CanPlay(i))
                player->Rename(i, i - 1);
        }
    }
    obj->erase(std::to_string(i-1));

    SaveBinds();
}

/* Utility function to convert int deviceIDs to corresponding device-typed objects */
device AudioManager::GetDeviceByIndex(int i)
{
    for (auto const& [dName, id] : deviceList)
    {
        if (id == i)
        {
            if (inputDevices.find(dName) != inputDevices.end())
            {
                return inputDevices[dName];
            }
            else if (outputDevices.find(dName) != outputDevices.end())
            {
                return outputDevices[dName];
            }
        }
    }

    for (auto const& [dName, deviceSettings] : loopbackDevices)
    {
        if (deviceSettings.id == i)
        {
            return loopbackDevices[dName];
        }
    }

    return {-1, -1};
}

void AudioManager::OverrideSound(std::string fname, int idx)
{
    std::string path = appdata + dirName + "samples/" + std::to_string(idx) + ".mp3";
    if (std::filesystem::exists(path))
    {
        std::filesystem::remove(path);
    }

    std::filesystem::path p = fname;
    if (p.extension() == "mp3")
    {
        std::filesystem::copy(fname, path);
    }
    else
    {
        SF_INFO info_nonMp3, info_mp3;
        SNDFILE* nonMp3 = sf_open(fname.c_str(), SFM_READ, &info_nonMp3);
        info_mp3.samplerate = info_nonMp3.samplerate;
        info_mp3.channels = info_nonMp3.channels;
        info_mp3.format = SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_III;
        SNDFILE* mp3 = sf_open(path.c_str(), SFM_WRITE, &info_mp3);

        sf_count_t count;
        float *in = new float[1024];
        while (true)
        {
            count = sf_read_float(nonMp3, in, 1024);
            sf_write_float(mp3, in, 1024);
            if (count < 1024) break;
        }

        sf_close(nonMp3);
        sf_close(mp3);
    }

    SF_INFO info;
    SNDFILE* infile = sf_open(path.c_str(), SFM_READ, &info);
    if (info.samplerate != settings["sampleRate"])
    {
        sf_count_t count;
        SF_INFO infoR = info;
        infoR.samplerate = settings["sampleRate"];
        SNDFILE* outfile = sf_open((appdata + dirName + "samples/" + std::to_string(idx) + "r.mp3").c_str(), SFM_WRITE, &infoR);
        double ratio = (double)settings["sampleRate"] / (double)info.samplerate;

        int err;
        SRC_STATE* src = src_new(SRC_SINC_FASTEST, info.channels, &err);
        SRC_DATA *dat = new SRC_DATA();
        float *in = new float[1024], *out = new float[(int)(512 * ratio) * 2 + 2];
        dat->data_in = in;
        dat->data_out = out;
        dat->input_frames = 1024;
        dat->output_frames = (int)((1024 * ratio)) + 2;
        dat->src_ratio = ratio;

        while (1)
        {
            count = sf_read_float(infile, in, 1024);
            if (count == 0) break;
            src_process(src, dat);
            sf_write_float(outfile, out, dat->output_frames_gen);
            if (count < 1024) break;
        }

        sf_close(infile);
        sf_close(outfile);
        src_delete(src);
        delete[] in;
        delete[] out;
        delete dat;

        std::filesystem::remove(path.c_str());
        std::filesystem::rename((appdata + dirName + "samples/" + std::to_string(idx) + "r.mp3").c_str(), path.c_str());
    }

    if (info.channels != 2) {
        SF_INFO info;
        SNDFILE* infile = sf_open(path.c_str(), SFM_READ, &info);

        sf_count_t count;
        SF_INFO infoR = info;
        infoR.channels = 2;
        SNDFILE* outfile = sf_open((appdata + dirName + "samples/" + std::to_string(idx) + "c.mp3").c_str(), SFM_WRITE, &infoR);
        double ratio = (double)settings["sampleRate"] / (double)info.samplerate;

        float *in = new float[512 * info.channels], *out = new float[1024];

        while (1)
        {
            count = sf_read_float(infile, in, 512 * info.channels);
            if (count == 0) break;
            for (int i = 0; i < 512; i++) {
                for (int j = 1; j < info.channels; j++) {
                    in[info.channels * i] += in[info.channels * i + j];
                }
                out[2 * i] = in[info.channels * i] / info.channels;
                out[2 * i + 1] = out[2 * i];
            }
            sf_write_float(outfile, out, 1024);
            if (count < 512 * info.channels) break;
        }

        sf_close(infile);
        sf_close(outfile);
        delete[] in;
        delete[] out;
        std::filesystem::remove(path.c_str());
        std::filesystem::rename((appdata + dirName + "samples/" + std::to_string(idx) + "c.mp3").c_str(), path.c_str());
    }
}

std::pair<bindType, std::string> AudioManager::isExistingKeybind(int keycode, std::string deviceName) {
    for (auto const& [k,v] : soundboardHotkeys.items())
        if (v["keycode"] == keycode && v["deviceName"] == deviceName)
            return {bindType::soundboard, k};

    for (auto const& [k,v] : voiceFXHotkeys.items())
        if (v["keycode"] == keycode && v["deviceName"] == deviceName)
            return {bindType::voicefx, k};

    if (settings["recordOverKeybind"] == keycode && settings["recordOverDeviceName"] == deviceName)
        return {bindType::recordOver, ""};

    return {bindType::invalid, ""};
}
