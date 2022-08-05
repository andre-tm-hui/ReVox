#include "audiomanager.h"

AudioManager::AudioManager()
{
    // initialize portaudio for the entire app
    Pa_Initialize();

    GetDeviceSettings();

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
            settings["virtualInputDevice"] = defVInput;
            settings["virtualOutputDevice"] = defVOutput;

            std::ifstream soundboardBindsFile(appdata + dirName + "soundboard.json");
            if (!soundboardBindsFile.fail()) soundboardBindsFile >> soundboardHotkeys;

            std::ifstream voiceFXBindsFile(appdata + dirName + "voicefx.json");
            if (!voiceFXBindsFile.fail()) voiceFXBindsFile >> voiceFXHotkeys;
        }
    }

    // start the portaudio stream objects
    loopbackRecorder    = new Recorder(     GetDeviceByIndex(settings["loopbackDevice"]),
                                            settings["sampleRate"].get<int>(),
                                            settings["framesPerBuffer"].get<int>(),
                                            std::string(appdata_c) + dirName, "l");
    player              = new Player(       GetDeviceByIndex(settings["virtualInputDevice"]),
                                            settings["sampleRate"].get<int>(),
                                            settings["framesPerBuffer"].get<int>(),
                                            std::string(appdata_c) + dirName);
    /*monitor             = new Player(       GetDeviceByIndex(settings["outputDevice"]),
                                            settings["sampleRate"].get<int>(),
                                            settings["framesPerBuffer"].get<int>(),
                                            std::string(appdata_c) + dirName);*/
    passthrough         = new Passthrough(  GetDeviceByIndex(settings["inputDevice"]),
                                            GetDeviceByIndex(settings["virtualInputDevice"]),
                                            settings["sampleRate"].get<int>(),
                                            settings["framesPerBuffer"].get<int>(),
                                            std::string(appdata_c) + dirName);

    WaitForReady();
    player->maxLiveSamples = settings["maxNumberOfSounds"];
    player->data.maxFileLength = settings["maxFileLength"];
}

AudioManager::~AudioManager()
{
    SaveSettings();
    SaveBinds();
}

void AudioManager::SetCheckboxes(std::map<std::string, QCheckBox*> *checkboxes)
{
    passthrough->checkboxes = checkboxes;
}

void AudioManager::WaitForReady()
{
    while (!loopbackRecorder->initialSetup ||
           !player->initialSetup ||
           !passthrough->initialSetup) { /*std::cout << "waiting" << std::endl;*/ }
}

void AudioManager::GetDeviceSettings()
{
    std::map<std::string, int> apiMap;

    // instantiate devices to default values
    settings["inputDevice"] = Pa_GetDefaultInputDevice();
    settings["outputDevice"] = Pa_GetDefaultOutputDevice();

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
                loopbackDevices[deviceName] = {Pa_HostApiDeviceIndexToDeviceIndex(apiMap["Windows WASAPI"], i), 1};
                // for default device setup
                if (deviceName.find(Pa_GetDeviceInfo(settings["outputDevice"])->name) != std::string::npos)
                {
                    settings["loopbackDevice"] = loopbackDevices[deviceName].id;
                }
            }
            else
            {
                // add the device to a list of devices if it's not a loopback
                deviceList[deviceName] = Pa_HostApiDeviceIndexToDeviceIndex(apiMap["Windows WASAPI"], i);
            }
        }
    }

    // if MME is available, iterate through all MME devices. MME is standard for Windows machines, and raises the least issues with portaudio streams
    if (apiMap.find("MME") != apiMap.end())
    {
        for (int i = 0; i < Pa_GetHostApiInfo(apiMap["MME"])->deviceCount; i++)
        {
            std::string deviceName = Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(apiMap["MME"], i))->name;
            // MME device names are limited to 32 characters, so we try to find a WASAPI device with a similar name, and change the device ID
            for (auto const& [name, id] : deviceList)
            {
                if (name.find(deviceName) != std::string::npos)
                {
                    deviceName = name;
                    break;
                }
            }
            deviceList[deviceName] = Pa_HostApiDeviceIndexToDeviceIndex(apiMap["MME"], i);
            // set the default virtual input device - assumes the user is using VB-Audio's Virtual Cable https://vb-audio.com/Cable/
            if (deviceName.find("CABLE Input") != std::string::npos)
            {
                settings["virtualInputDevice"] = deviceList[deviceName];
                defVInput = deviceList[deviceName];
            }
            else if (deviceName.find("CABLE Output") != std::string::npos)
            {
                settings["virtualOutputDevice"] = deviceList[deviceName];
                defVOutput = deviceList[deviceName];
            }
        }
    }

    // start gathering the capabilities of each audio device
    PaStreamParameters params;
    params.hostApiSpecificStreamInfo = NULL;
    params.sampleFormat = PA_SAMPLE_TYPE;

    PaError err;
    bool isInput = true;
    for (auto const& [name, id] : deviceList)
    {
        // check if this device is an input or output device
        device deviceSettings;
        params.device = id;
        params.channelCount = 1;
        params.suggestedLatency = Pa_GetDeviceInfo(id)->defaultLowInputLatency;
        deviceSettings.id = id;
        deviceSettings.nChannels = params.channelCount;

        // assumes most modern devices support at least a 44100Hz sample rate
        err = Pa_IsFormatSupported(&params, NULL, 44100);
        if (err != 0)
        {
            err = Pa_IsFormatSupported(NULL, &params, 44100);
            if (err == 0)
            {
                isInput = false;
                outputDevices[name] = deviceSettings;
            }
        }
        else
        {
            isInput = true;
            inputDevices[name] = deviceSettings;
        }

        // check the maximum number of channels available on the device - needs to be done to avoid problems with incorrectly initialized portaudio streams
        while (true)
        {
            params.channelCount++;
            err = Pa_IsFormatSupported(isInput ? &params : NULL, isInput ? NULL : &params, 44100);
            // capped at 16 to avoid anomalous audio devices with 1000+ channels from slowing down init time
            if (err != 0 || params.channelCount > 16)
            {
                break;
            }
        }
        if (isInput)
        {
            inputDevices[name].nChannels = params.channelCount - 1;
            //fprintf(stdout, "Input Device: %s with %d channels\n", name.c_str(), inputDevices[name].nChannels); fflush(stdout);
        }
        else
        {
            outputDevices[name].nChannels = params.channelCount - 1;
            //fprintf(stdout, "Output Device: %s with %d channels\n", name.c_str(), outputDevices[name].nChannels); fflush(stdout);

            // assume that the loopback devices shares the same number of channels as it's corresponding output device
            if (loopbackDevices.find(name + " [Loopback]") != loopbackDevices.end())
            {
                loopbackDevices[name + " [Loopback]"].nChannels = params.channelCount - 1;
                //fprintf(stdout, "Output Device: %s [Loopback] with %d channels\n", name.c_str(), loopbackDevices[name + " [Loopback]"].nChannels); fflush(stdout);
            }
        }
    }
}

/* Reset functions for portaudio objects, called when corresponding devices are changed in the GUI */
void AudioManager::ResetLoopbackRecorder()
{
    delete loopbackRecorder;
    loopbackRecorder = new Recorder(GetDeviceByIndex(settings["loopbackDevice"]),
                                    settings["sampleRate"].get<int>(),
                                    settings["framesPerBuffer"].get<int>(),
                                    appdata + dirName, "l");
    SaveSettings();
}

void AudioManager::ResetPlayer()
{
    delete player;
    player = new Player(GetDeviceByIndex(settings["virtualInputDevice"]),
                        settings["sampleRate"].get<int>(),
                        settings["framesPerBuffer"].get<int>(),
                        appdata + dirName);
    SaveSettings();
}

void AudioManager::ResetMonitor()
{
    /*delete monitor;
    monitor = new Player(GetDeviceByIndex(settings["outputDevice"]),
                         settings["sampleRate"].get<int>(),
                         settings["framesPerBuffer"].get<int>(),
                         appdata + dirName);*/
    SaveSettings();
}

void AudioManager::ResetPassthrough()
{
    delete passthrough;
    passthrough = new Passthrough(GetDeviceByIndex(settings["inputDevice"]),
                                  GetDeviceByIndex(settings["virtualInputDevice"]),
                                  settings["sampleRate"].get<int>(),
                                  settings["framesPerBuffer"].get<int>(),
                                  appdata + dirName);
    SaveSettings();
}

void AudioManager::Reset(int input, int output, int loopback)
{
    Pa_Terminate();
    Pa_Initialize();
    settings["inputDevice"] = input;
    settings["outputDevice"] = output;
    settings["loopbackDevice"] = loopback;

    ResetLoopbackRecorder();
    ResetPlayer();
    //ResetMonitor();
    ResetPassthrough();

    WaitForReady();
}

/* Configuration save functions */
void AudioManager::SaveSettings()
{
    std::ofstream settingsFile(appdata + dirName + "settings.json");
    settingsFile << std::setw(4) << settings << std::endl;
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

        if (!isSoundboard)
        {
            (*obj)[std::to_string(keycode)]["reverb"]["enabled"] = passthrough->data.reverb->getEnabled();
            (*obj)[std::to_string(keycode)]["reverb"]["roomsize"] = passthrough->data.reverb->getroomsize();
            (*obj)[std::to_string(keycode)]["reverb"]["mix"] = passthrough->data.reverb->getwet();
            (*obj)[std::to_string(keycode)]["reverb"]["width"] = passthrough->data.reverb->getwidth();
            (*obj)[std::to_string(keycode)]["reverb"]["damp"] = passthrough->data.reverb->getdamp();

            (*obj)[std::to_string(keycode)]["autotune"]["enabled"] = passthrough->data.pitchShift->getAutotune();
            (*obj)[std::to_string(keycode)]["autotune"]["speed"] = 1.f;//passthrough->data.autotune->getspeed();
            (*obj)[std::to_string(keycode)]["autotune"]["notes"] = passthrough->data.pitchShift->getNotes();

            (*obj)[std::to_string(keycode)]["pitch"]["enabled"] = passthrough->data.pitchShift->getPitchshift();
            (*obj)[std::to_string(keycode)]["pitch"]["pitch"] = passthrough->data.pitchShift->getPitchscale();
        }
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

void AudioManager::OverrideConfig(std::string keycode)
{
    voiceFXHotkeys[keycode]["reverb"]["enabled"] = passthrough->data.reverb->getEnabled();
    voiceFXHotkeys[keycode]["reverb"]["roomsize"] = passthrough->data.reverb->getroomsize();
    voiceFXHotkeys[keycode]["reverb"]["mix"] = passthrough->data.reverb->getwet();
    voiceFXHotkeys[keycode]["reverb"]["width"] = passthrough->data.reverb->getwidth();
    voiceFXHotkeys[keycode]["reverb"]["damp"] = passthrough->data.reverb->getdamp();

    voiceFXHotkeys[keycode]["autotune"]["enabled"] = passthrough->data.pitchShift->getAutotune();
    voiceFXHotkeys[keycode]["autotune"]["speed"] = 1.f;//passthrough->data.autotune->getspeed();
    voiceFXHotkeys[keycode]["autotune"]["notes"] = passthrough->data.pitchShift->getNotes();

    voiceFXHotkeys[keycode]["pitch"]["enabled"] = passthrough->data.pitchShift->getPitchshift();
    voiceFXHotkeys[keycode]["pitch"]["pitch"] = passthrough->data.pitchShift->getPitchscale();

    SaveBinds();
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
