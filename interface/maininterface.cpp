#include "maininterface.h"

MainInterface::MainInterface() : BaseInterface("")
{
    Pa_Initialize();

    char* appdata_c = std::getenv("APPDATA");
    if (appdata_c) {
        settings = defaultSettings;
        rootDir = std::string(appdata_c) + ROOTNAME;
        dataPath = "settings.json";
        if (!std::filesystem::exists(rootDir))
            std::filesystem::create_directories(rootDir + "samples");
        LoadSettings();
        if (settings["outputDevice"] == "No Device Detected") {
            UpdateSettings<std::string>("outputDevice", Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->name);
        }
    } else {
        std::cout<<"failed"<<std::endl;
        delete this;
    }

    soundboardManager.reset(new SoundboardManager(rootDir, settings["sampleRate"].get<int>()));
    fxManager.reset(new FXManager(rootDir, settings["framesPerBuffer"], settings["sampleRate"]));

    GetDeviceSettings();
    SetupStreams();
}

MainInterface::~MainInterface() {
    delete[] inputBuffer;
    delete[] playbackBuffer;
}

void MainInterface::SetCurrentOutputDevice(int id) {
    ids.output = id;
    UpdateSettings<std::string>("outputDevice", Pa_GetDeviceInfo(ids.output)->name);
    Reset();
}

void MainInterface::KeyEvent(int keycode, std::string deviceName, int event) {
    bool soundboardExists = soundboardManager->KeybindExists(keycode, deviceName),
         fxExists = fxManager->KeybindExists(keycode, deviceName);
    if (soundboardManager->IsRebinding()) {
        if (!soundboardExists && !fxExists) soundboardManager->KeyEvent(keycode, deviceName, event);
        return;
    }
    if (fxManager->IsRebinding()) {
        if (!soundboardExists && !fxExists) fxManager->KeyEvent(keycode, deviceName, event);
        return;
    }
    if (blocked) return;
    if (soundboardExists) {
        soundboardManager->KeyEvent(keycode, deviceName, event);
        return;
    }
    if (fxExists) {
        fxManager->KeyEvent(keycode, deviceName, event);
        return;
    }
}

void MainInterface::WaitForReady()
{
    while (!monitor->initialSetup ||
           !player->initialSetup ||
           !passthrough->initialSetup ||
           !noiseGen->initialSetup) {}

    if (cleanOutput != nullptr) while (!cleanOutput->initialSetup) {}
}

void MainInterface::GetDeviceSettings()
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

int MainInterface::GetChannels(int id, bool isInput)
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

int MainInterface::GetCorrespondingLoopbackDevice(int i)
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
void MainInterface::Reset(bool devicesChanged)
{
    ResetStreams();

    Pa_Terminate();
    Pa_Initialize();

    GetDeviceSettings();

    SetupStreams();

    WaitForReady();

    if (!devicesChanged) { SaveSettings(); }
}

void MainInterface::ResetStreams() {
    soundboardManager->ResetStreams();
    fxManager->ResetStreams();
    noiseGen.reset();
    monitor.reset();
    player.reset();
    passthrough.reset();
    cleanOutput.reset();
}

void MainInterface::SetupStreams()
{
    inputBuffer = new float[settings["framesPerBuffer"].get<int>() * 2 * 3];
    playbackBuffer = new float[settings["framesPerBuffer"].get<int>() * 2 * 3];
    int loopbackdevice = GetCorrespondingLoopbackDevice(ids.streamOut);

    noiseGen.reset(       new NoiseGenerator(GetDeviceByIndex(ids.streamOut),
                                             settings["sampleRate"].get<int>()));
    monitor.reset(        new Monitor(       GetDeviceByIndex(loopbackdevice),
                                             GetDeviceByIndex(ids.output),
                                             settings["sampleRate"].get<int>(),
                                             settings["framesPerBuffer"].get<int>(),
                                             rootDir,
                                             inputBuffer, playbackBuffer));
    player.reset(         new Player(        GetDeviceByIndex(ids.vInput),
                                             settings["sampleRate"].get<int>(),
                                             settings["framesPerBuffer"].get<int>(),
                                             rootDir,
                                             playbackBuffer));
    passthrough.reset(    new Passthrough(   GetDeviceByIndex(ids.input),
                                             GetDeviceByIndex(ids.vInput),
                                             settings["sampleRate"].get<int>(),
                                             settings["framesPerBuffer"].get<int>(),
                                             rootDir,
                                             inputBuffer));
    if (ids.streamOut != ids.output)
        cleanOutput.reset( new CleanOutput(  GetDeviceByIndex(loopbackdevice),
                                             GetDeviceByIndex(ids.output),
                                             settings["sampleRate"].get<int>(),
                                             1));
    else
        cleanOutput.reset();

    soundboardManager->SetStreams(passthrough, monitor, player);
    fxManager->SetStreams(passthrough, monitor);


    //fxManager->SetHUD(this->hud);
}

/* Utility function to convert int deviceIDs to corresponding device-typed objects */
device MainInterface::GetDeviceByIndex(int i)
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
