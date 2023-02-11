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
    delete inputQueue;
    delete playbackQueue;
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
    if (std::string("Windows WASAPI").compare(Pa_GetHostApiInfo(Pa_GetDefaultHostApi())->name) != 0)
        return;

    ids = {-1, -1, -1, -1};
    deviceList = {};
    inputDevices = {};
    outputDevices = {};
    loopbackDevices = {};

    for (int i = 0; i < Pa_GetDeviceCount(); i++)
    {
        // separate normal devices from loopback devices
        std::string deviceName = Pa_GetDeviceInfo(i)->name;
        if (deviceName.find("[Loopback]") != std::string::npos)
        {
            loopbackDevices[deviceName] = {i, 2};
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
            deviceList[deviceName] = i;

            // set the default virtual input device - assumes the user is using VB-Audio's Virtual Cable https://vb-audio.com/Cable/
            if (deviceName.find("CABLE Input") != std::string::npos)
            {
                ids.vInput = deviceList[deviceName];
            }
            else if (deviceName.find("CABLE Output") != std::string::npos)
            {
                ids.vOutput = deviceList[deviceName];
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
    fxManager->SetFramesPerBuffer(settings["framesPerBuffer"].get<int>());

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
    inputQueue = new std::queue<float>();
    playbackQueue = new std::queue<float>();
    int loopbackdevice = GetCorrespondingLoopbackDevice(ids.streamOut);

    noiseGen.reset(       new NoiseGenerator(GetDeviceByIndex(ids.streamOut),
                                             settings["sampleRate"].get<int>()));
    monitor.reset(        new Monitor(       GetDeviceByIndex(ids.input),
                                             GetDeviceByIndex(ids.output),
                                             settings["sampleRate"].get<int>(),
                                             settings["framesPerBuffer"].get<int>(),
                                             rootDir,
                                             inputQueue, playbackQueue));
    player.reset(         new Player(        GetDeviceByIndex(loopbackdevice),
                                             GetDeviceByIndex(ids.vInput),
                                             settings["sampleRate"].get<int>(),
                                             settings["framesPerBuffer"].get<int>(),
                                             rootDir,
                                             playbackQueue));
    passthrough.reset(    new Passthrough(   GetDeviceByIndex(ids.input),
                                             GetDeviceByIndex(ids.vInput),
                                             settings["sampleRate"].get<int>(),
                                             settings["framesPerBuffer"].get<int>(),
                                             rootDir,
                                             inputQueue));
    if (ids.streamOut != ids.output)
        cleanOutput.reset( new CleanOutput(  GetDeviceByIndex(loopbackdevice),
                                             GetDeviceByIndex(ids.output),
                                             settings["sampleRate"].get<int>(),
                                             1));
    else
        cleanOutput.reset();

    soundboardManager->SetStreams(passthrough, monitor, player);
    fxManager->SetStreams(passthrough, monitor);
}

/* Utility function to convert int deviceIDs to corresponding device-typed objects */
device MainInterface::GetDeviceByIndex(int i)
{
    for (auto const& [dName, id] : deviceList)
    {
        if (id != i) continue;

        if (inputDevices.find(dName) != inputDevices.end())
        {
            return inputDevices[dName];
        }
        else if (outputDevices.find(dName) != outputDevices.end())
        {
            return outputDevices[dName];
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
