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

            SaveSettings();
        }
        // otherwise, load saved configurations
        else
        {
            std::ifstream settingsFile (appdata + dirName + "settings.json");
            if (!settingsFile.fail()) settingsFile >> settings;

            std::ifstream soundboardBindsFile(appdata + dirName + "soundboard.json");
            if (!soundboardBindsFile.fail()) soundboardBindsFile >> soundboardHotkeys;

            std::ifstream voiceFXBindsFile(appdata + dirName + "voicefx.json");
            if (!voiceFXBindsFile.fail()) voiceFXBindsFile >> voiceFXHotkeys;
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

void AudioManager::Record(int keycode)
{
    if ((soundboardHotkeys[std::to_string(keycode)]["recordInput"] ||
         soundboardHotkeys[std::to_string(keycode)]["recordLoopback"]) &&
            std::filesystem::exists(appdata + dirName + "samples/" + std::to_string(keycode) + ".mp3"))
        std::filesystem::remove(appdata + dirName + "samples/" + std::to_string(keycode) + ".mp3");

    if (soundboardHotkeys[std::to_string(keycode)]["recordInput"] && passthrough != nullptr) passthrough->Record(keycode);
    if (soundboardHotkeys[std::to_string(keycode)]["recordLoopback"] && monitor != nullptr) monitor->Record(keycode);

    recording = true;
}

void AudioManager::StopRecording()
{
    if (passthrough != nullptr) passthrough->Stop();
    if (monitor != nullptr) monitor->Stop();

    recording = false;
    wv->SetAudioClip();
}

void AudioManager::Play(int keycode, bool recordFallback)
{
    if (player != nullptr)
    {
        if (player->CanPlay(keycode))
            player->Play(keycode, soundboardHotkeys[std::to_string(keycode)]);
        else if (recordFallback)
            Record(keycode);
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
                if (outputDeviceName.find(settings["outputDevice"]) != std::string::npos)
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
    delete passthrough;
    delete player;
    delete monitor;
    delete cleanOutput;
    delete noiseGen;

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
void AudioManager::Rebind(int keycode)
{
    rebindAt = keycode;
}

void AudioManager::SetNewBind(int keycode, bool isSoundboard)
{
    json *obj, entry;
    if (isSoundboard)
    {
        obj = &soundboardHotkeys;
        entry = baseSoundboardHotkey;
    }
    else
    {
        obj = &voiceFXHotkeys;
        entry = baseFXHotkey;
    }

    if (rebindAt != -1)
    {
        (*obj)[std::to_string(keycode)] = (*obj)[std::to_string(rebindAt)];
        if (isSoundboard && player->CanPlay(rebindAt)) player->Rename(rebindAt, keycode);

        obj->erase(std::to_string(rebindAt));
        rebindAt = -1;
    }
    else
    {
        (*obj)[std::to_string(keycode)] = entry;
    }
    SaveBinds();
}

void AudioManager::RemoveBind(int keycode)
{
    if (soundboardHotkeys.find(std::to_string(keycode)) != soundboardHotkeys.end())
    {
        soundboardHotkeys.erase(std::to_string(keycode));
    }
    else if (voiceFXHotkeys.find(std::to_string(keycode)) != voiceFXHotkeys.end())
    {
        voiceFXHotkeys.erase(std::to_string(keycode));
    }
    if (std::filesystem::exists(appdata + dirName + "samples/" + std::to_string(keycode) + ".mp3"))
    {
        std::filesystem::remove(appdata + dirName + "samples/" + std::to_string(keycode) + ".mp3");
    }
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

void AudioManager::OverrideSound(std::string fname, int keycode)
{
    std::string path = appdata + dirName + "samples/" + std::to_string(keycode) + ".mp3";
    if (std::filesystem::exists(path))
    {
        std::filesystem::remove(path);
    }
    std::filesystem::copy(fname, path);

    SF_INFO info;
    SNDFILE* infile = sf_open(path.c_str(), SFM_READ, &info);
    if (info.samplerate != settings["sampleRate"])
    {
        sf_count_t count;
        SF_INFO infoR = info;
        infoR.samplerate = settings["sampleRate"];
        SNDFILE* outfile = sf_open((appdata + dirName + "samples/" + std::to_string(keycode) + "r.mp3").c_str(), SFM_WRITE, &infoR);
        double ratio = (double)settings["sampleRate"] / (double)info.samplerate;

        int err;
        SRC_STATE* src = src_new(SRC_SINC_FASTEST, info.channels, &err);
        SRC_DATA *dat = new SRC_DATA();
        float *in = new float[1024], *out = new float[(int)(512 * ratio) * 2 + 2];
        dat->data_in = in;
        dat->data_out = out;
        dat->input_frames = 1024 / info.channels;
        dat->output_frames = (int)((512 * ratio)) + 1;
        dat->src_ratio = ratio;

        while (1)
        {
            count = sf_read_float(infile, in, 1024);
            if (count == 0) break;
            src_process(src, dat);
            sf_write_float(outfile, out, dat->output_frames_gen * 2);
            if (count < 1024) break;
        }

        sf_close(infile);
        sf_close(outfile);
        src_delete(src);
        delete[] in;
        delete[] out;
        delete dat;

        std::filesystem::remove(path.c_str());
        std::filesystem::rename((appdata + dirName + "samples/" + std::to_string(keycode) + "r.mp3").c_str(), path.c_str());
    }
}
