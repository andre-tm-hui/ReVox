#include "audiomanager.h"

AudioManager::AudioManager()
{
    // initialize portaudio for the entire app
    Pa_Initialize();

    // instantiate devices to default values
    inputDevice = Pa_GetDefaultInputDevice();
    outputDevice = Pa_GetDefaultOutputDevice();

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
                if (deviceName.find(Pa_GetDeviceInfo(outputDevice)->name) != std::string::npos)
                {
                    loopbackDevice = loopbackDevices[deviceName].id;
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
                virtualInputDevice = deviceList[deviceName];
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

    // get the %appdata% location - all Windows machines should pass the appdata_c check
    char* appdata_c = std::getenv("APPDATA");
    if (appdata_c)
    {
        appdata = appdata_c;
        // if a folder for the app does not exist, create the folder and it's subdirectories, and initialize some default configuration files
        if (!std::filesystem::exists(appdata + dirName))
        {
            std::filesystem::create_directory(appdata + dirName);
            std::filesystem::create_directory(appdata + dirName + "samples/");

            this->sampleRate = 48000;
            this->framesPerBuffer = 128;

            // start the portaudio objects
            inputDeviceRecorder = new Recorder(GetDeviceByIndex(inputDevice), this->sampleRate, this->framesPerBuffer, std::string(appdata_c) + dirName);
            loopbackRecorder = new Recorder(GetDeviceByIndex(loopbackDevice), this->sampleRate, this->framesPerBuffer, std::string(appdata_c) + dirName, "l");
            player = new Player(GetDeviceByIndex(virtualInputDevice), this->sampleRate, this->framesPerBuffer, std::string(appdata_c) + dirName);
            monitor = new Player(GetDeviceByIndex(outputDevice), this->sampleRate, this->framesPerBuffer, std::string(appdata_c) + dirName);
            passthrough = new Passthrough(GetDeviceByIndex(inputDevice), GetDeviceByIndex(virtualInputDevice), this->sampleRate, this->framesPerBuffer);

            for (int i = 96; i < 106; i++)
            {
                SetNewBind(i);
            }

            SaveSettings();
            SaveBinds();
        }
        // otherwise, load saved configurations
        else
        {
            std::ifstream settingsFile (appdata + dirName + "settings.cfg");
            std::string line, settingText, settingValue;;

            if (settingsFile.is_open())
            {
                while (std::getline(settingsFile, line))
                {
                    int equal = line.find("=");
                    settingText = line.substr(0, equal);
                    settingValue = line.substr(equal + 1);

                    if (settingText.compare("inputDevice") == 0) { this->inputDevice = std::stoi(settingValue); }
                    else if (settingText.compare("outputDevice") == 0) { this->outputDevice = std::stoi(settingValue); }
                    else if (settingText.compare("loopbackDevice") == 0) { this->loopbackDevice = std::stoi(settingValue); }
                    else if (settingText.compare("virtualInputDevice") == 0) { this->virtualInputDevice = std::stoi(settingValue); }
                    else if (settingText.compare("sampleRate") == 0) { this->sampleRate = std::stoi(settingValue); }
                    else if (settingText.compare("framesPerBuffer") == 0) { this->framesPerBuffer = std::stoi(settingValue); fprintf(stdout, "%d\n", this->framesPerBuffer); fflush(stdout);}
                }
                settingsFile.close();
            }

            std::ifstream bindsFile (appdata + dirName + "binds.cfg");
            int currentBind;
            int reverbBind = -1;

            if (bindsFile.is_open())
            {
                while (std::getline(bindsFile, line))
                {
                    int equal = line.find("=");
                    settingText = line.substr(0, equal);
                    settingValue = line.substr(equal + 1);

                    if (settingText.compare("bind") == 0) {
                        if (this->keybinds[currentBind].type == 1)
                        {
                            if (this->keybinds[currentBind].fxType == 0)
                            {
                                reverbBind = currentBind;
                            }
                        }
                        currentBind = std::stoi(settingValue); this->keybinds[currentBind] = {};
                    }
                    else if (settingText.compare("type") == 0) { this->keybinds[currentBind].type = std::stoi(settingValue); }
                    else if (settingText.compare("recordInput") == 0) { this->keybinds[currentBind].recordInput = settingValue.compare("true") == 0 ? true : false; }
                    else if (settingText.compare("recordLoopback") == 0) { this->keybinds[currentBind].recordLoopback = settingValue.compare("true") == 0 ? true : false; }
                    else if (settingText.compare("padAudio") == 0) { this->keybinds[currentBind].padAudio = settingValue.compare("true") == 0 ? true : false; }
                    else if (settingText.compare("fxType") == 0) { this->keybinds[currentBind].fxType = std::stoi(settingValue); }
                    else if (settingText.compare("roomsize") == 0) { this->keybinds[currentBind].roomsize = std::stof(settingValue); }
                    else if (settingText.compare("damp") == 0) { this->keybinds[currentBind].damp = std::stof(settingValue); }
                    else if (settingText.compare("width") == 0) { this->keybinds[currentBind].width = std::stof(settingValue); }
                    else if (settingText.compare("wet") == 0) { this->keybinds[currentBind].wet = std::stof(settingValue); }
                    else if (settingText.compare("dry") == 0) { this->keybinds[currentBind].dry = std::stof(settingValue); }
                }
                bindsFile.close();
            }

            // start the portaudio objects
            inputDeviceRecorder = new Recorder(GetDeviceByIndex(inputDevice), this->sampleRate, this->framesPerBuffer, std::string(appdata_c) + dirName);
            loopbackRecorder = new Recorder(GetDeviceByIndex(loopbackDevice), this->sampleRate, this->framesPerBuffer, std::string(appdata_c) + dirName, "l");
            player = new Player(GetDeviceByIndex(virtualInputDevice), this->sampleRate, this->framesPerBuffer, std::string(appdata_c) + dirName);
            monitor = new Player(GetDeviceByIndex(outputDevice), this->sampleRate, this->framesPerBuffer, std::string(appdata_c) + dirName);
            passthrough = new Passthrough(GetDeviceByIndex(inputDevice), GetDeviceByIndex(virtualInputDevice), this->sampleRate, this->framesPerBuffer);

            if (reverbBind != -1)
            {
                passthrough->data.reverb->setroomsize(this->keybinds[reverbBind].roomsize);
                passthrough->data.reverb->setdamp(this->keybinds[reverbBind].damp);
                passthrough->data.reverb->setwidth(this->keybinds[reverbBind].width);
                passthrough->data.reverb->setwet(this->keybinds[reverbBind].wet);
                passthrough->data.reverb->setdry(this->keybinds[reverbBind].dry);
            }
        }
    }


}

AudioManager::~AudioManager()
{
    SaveSettings();
    SaveBinds();
}

/* Reset functions for portaudio objects, called when corresponding devices are changed in the GUI */
void AudioManager::ResetInputRecorder()
{
    delete inputDeviceRecorder;
    inputDeviceRecorder = new Recorder(GetDeviceByIndex(inputDevice), sampleRate, framesPerBuffer, appdata + dirName);
    SaveSettings();
}

void AudioManager::ResetLoopbackRecorder()
{
    delete loopbackRecorder;
    loopbackRecorder = new Recorder(GetDeviceByIndex(loopbackDevice), sampleRate, framesPerBuffer, appdata + dirName);
    SaveSettings();
}

void AudioManager::ResetPlayer()
{
    delete player;
    player = new Player(GetDeviceByIndex(virtualInputDevice), sampleRate, framesPerBuffer, appdata + dirName);
    SaveSettings();
}

void AudioManager::ResetMonitor()
{
    delete monitor;
    monitor = new Player(GetDeviceByIndex(outputDevice), sampleRate, framesPerBuffer, appdata + dirName);
    SaveSettings();
}

void AudioManager::ResetPassthrough()
{
    delete passthrough;
    passthrough = new Passthrough(GetDeviceByIndex(inputDevice), GetDeviceByIndex(virtualInputDevice), sampleRate, framesPerBuffer);
    SaveSettings();
}

void AudioManager::Reset()
{
    Pa_Terminate();
    Pa_Initialize();
    ResetInputRecorder();
    ResetLoopbackRecorder();
    ResetPlayer();
    ResetPassthrough();
}

/* Configuration save functions */
void AudioManager::SaveSettings()
{
    std::ofstream settingsFile;
    settingsFile.open(appdata + dirName + "settings.cfg", std::ios::out);
    settingsFile << "inputDevice=" + std::to_string(inputDevice) + "\n";
    settingsFile << "outputDevice=" + std::to_string(outputDevice) + "\n";
    settingsFile << "loopbackDevice=" + std::to_string(loopbackDevice) + "\n";
    settingsFile << "virtualInputDevice=" + std::to_string(virtualInputDevice) + "\n";
    settingsFile << "sampleRate=" + std::to_string(this->sampleRate) + "\n";
    settingsFile << "framesPerBuffer=" + std::to_string(this->framesPerBuffer) + "\n";
    settingsFile.close();
}

void AudioManager::SaveBinds()
{
    std::ofstream bindsFile;
    bindsFile.open(appdata + dirName + "binds.cfg");
    for (auto const& [bind, options] : this->keybinds)
    {
        bindsFile << "bind=" + std::to_string(bind) + "\n";
        bindsFile << "type=" + std::to_string(options.type) + "\n";
        bindsFile << "recordInput=" + std::string(options.recordInput ? "true" : "false") + "\n";
        bindsFile << "recordLoopback=" + std::string(options.recordLoopback ? "true" : "false") + "\n";
        bindsFile << "padAudio=" + std::string(options.padAudio ? "true" : "false") + "\n";
        bindsFile << "fxType=" + std::to_string(options.fxType) + "\n";
        bindsFile << "roomsize=" + std::to_string(options.roomsize) + "\n";
        bindsFile << "damp=" + std::to_string(options.damp) + "\n";
        bindsFile << "width=" + std::to_string(options.width) + "\n";
        bindsFile << "wet=" + std::to_string(options.wet) + "\n";
        bindsFile << "dry=" + std::to_string(options.dry) + "\n";

        fprintf(stdout, "%f\n", options.roomsize); fflush(stdout);
        fprintf(stdout, "%f\n", options.damp); fflush(stdout);
        fprintf(stdout, "%f\n", options.width); fflush(stdout);
        fprintf(stdout, "%f\n", options.wet); fflush(stdout);
        fprintf(stdout, "%f\n", options.dry); fflush(stdout);
    }
    bindsFile.close();
}

/* Functions handling hotkey binding/rebinding */
void AudioManager::Rebind(int keycode)
{
    rebindAt = keycode;
}

void AudioManager::SetNewBind(int keycode)
{
    if (rebindAt != -1)
    {
        keybinds[keycode] = keybinds[rebindAt];

        if (player->CanPlay(rebindAt))
        {
            player->Rename(rebindAt, keycode);
        }

        keybinds.erase(rebindAt);
        rebindAt = -1;
    }
    else
    {
        keybinds[keycode] = {
            0,
            true,
            true,
            true,
            0,
            passthrough->data.reverb->getroomsize(),
            passthrough->data.reverb->getdamp(),
            passthrough->data.reverb->getwidth(),
            passthrough->data.reverb->getwet(),
            passthrough->data.reverb->getdry()
        };


    }
    SaveBinds();
}

void AudioManager::RemoveBind(int keycode)
{
    keybinds.erase(keycode);
    if (std::filesystem::exists(appdata + dirName + "samples/" + std::to_string(keycode) + ".wav"))
    {
        std::filesystem::remove(appdata + dirName + "samples/" + std::to_string(keycode) + ".wav");
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
