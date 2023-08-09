#include "maininterface.h"

MainInterface::MainInterface()
    : BaseInterface(""), LoggableObject("MainInterface") {
  Pa_Initialize();

  char* appdata_c = std::getenv("APPDATA");
  if (appdata_c) {
    settings = defaultSettings;
    rootDir = std::string(appdata_c) + ROOTNAME;

    try {
      auto l = spdlog::rotating_logger_mt<spdlog::async_factory>(
          "logger", rootDir + "logs/logs.txt", 1024 * 1024 * 5, 3);
      spdlog::flush_on(spdlog::level::warn);
      spdlog::flush_every(std::chrono::seconds(5));
    } catch (const spdlog::spdlog_ex& ex) {
      std::cout << "Log init failed: " << ex.what() << std::endl;
    }

    dataPath = "settings.json";
    if (!std::filesystem::exists(rootDir + "samples"))
      std::filesystem::create_directories(rootDir + "samples");
    LoadSettings();
    log(INFO, "Settings loaded");
    if (settings["outputDevice"] == "No Device Detected") {
      UpdateSettings<std::string>(
          "outputDevice", Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->name);
      log(WARN, "ReVox default device missing, falling back to system default");
    }
  } else {
    std::cout << "failed" << std::endl;
    delete this;
  }

  soundboardManager.reset(
      new SoundboardManager(rootDir, settings["sampleRate"].get<int>()));
  fxManager.reset(new FXManager(rootDir, settings["framesPerBuffer"],
                                settings["sampleRate"]));

  GetDeviceSettings();
  SetupStreams();
  log(INFO, "MainInterface setup");
}

MainInterface::~MainInterface() {
  delete inputQueue;
  delete playbackQueue;

  spdlog::drop_all();
}

void MainInterface::SetCurrentOutputDevice(int id) {
  log(INFO, "Changing output device");
  ids.output = id;
  UpdateSettings<std::string>("outputDevice",
                              Pa_GetDeviceInfo(ids.output)->name);
  Reset();
  log(INFO, "Output device changed");
}

bool MainInterface::KeyEvent(int keycode, std::string deviceName, int event) {
  bool detectKeyboard = settings["detectKeyboard"].get<bool>(),
       soundboardExists = soundboardManager->KeybindExists(keycode, deviceName,
                                                           detectKeyboard),
       fxExists = fxManager->KeybindExists(keycode, deviceName, detectKeyboard);
  if (soundboardManager->IsRebinding()) {
    if (!soundboardExists && !fxExists) {
      soundboardManager->KeyEvent(keycode, deviceName, event);
    }
    return true;
  }
  if (fxManager->IsRebinding()) {
    if (!soundboardExists && !fxExists) {
      fxManager->KeyEvent(keycode, deviceName, event);
    }
    return true;
  }
  if (blocked) return false;
  if (soundboardExists) {
    soundboardManager->KeyEvent(keycode, deviceName, event);
    return true;
  }
  if (fxExists) {
    fxManager->KeyEvent(keycode, deviceName, event);
    return true;
  }
  return false;
}

void MainInterface::WaitForReady() {
  while (!monitor->initialSetup || !player->initialSetup ||
         !passthrough->initialSetup || !noiseGen->initialSetup) {
  }

  if (cleanOutput != nullptr)
    while (!cleanOutput->initialSetup) {
    }
  log(INFO, "MainInterface ready");
}

void MainInterface::GetDeviceSettings() {
  log(INFO, "Getting device settings");

  ids = {-1, -1, -1, -1};
  deviceList = {};
  inputDevices = {};
  outputDevices = {};
  loopbackDevices = {};

  for (int i = 0; i < Pa_GetDeviceCount(); i++) {
      auto deviceInfo = Pa_GetDeviceInfo(i);
      std::string deviceName = deviceInfo->name;
      std::string truncatedName = deviceName.substr(0, 31);
      if (deviceName.find("Loopback") != std::string::npos) {
          // is a output loopback device
          std::string outputDeviceName =
              deviceName.substr(0, deviceName.size() - 11);
          outputDevices[outputDeviceName].ids["Loopback"] = i;
          deviceList[i] = &outputDevices[outputDeviceName];
      }
      // TODO: convert below into separate function(&map, truncatedName, deviceName, i, deviceInfo)
      else if (deviceInfo->maxInputChannels > 0) {
          // is an input device
          if (deviceName.length() >= 32 && inputDevices.find(truncatedName) != inputDevices.end()) {
              inputDevices[deviceName] = inputDevices[truncatedName];
              deviceList[inputDevices[truncatedName].ids["MME"]] = &inputDevices[deviceName];
              inputDevices.erase(truncatedName);
              inputDevices[deviceName].ids[Pa_GetHostApiInfo(deviceInfo->hostApi)->name] = i;
              
          }
          else {
              inputDevices[deviceName] = {
                  {{Pa_GetHostApiInfo(deviceInfo->hostApi)->name, i}},
                  deviceInfo->maxInputChannels
              };
          }
          deviceList[i] = &inputDevices[deviceName];
      }
      else if (deviceInfo->maxOutputChannels > 0) {
          // is an output device
          if (deviceName.length() >= 32 && outputDevices.find(truncatedName) != outputDevices.end()) {
              outputDevices[deviceName] = outputDevices[truncatedName];
              deviceList[outputDevices[truncatedName].ids["MME"]] = &outputDevices[deviceName];
              outputDevices[deviceName].ids[Pa_GetHostApiInfo(deviceInfo->hostApi)->name] = i;
              outputDevices.erase(truncatedName);
          }
          else {
              outputDevices[deviceName] = {
                  {{Pa_GetHostApiInfo(deviceInfo->hostApi)->name, i}},
                  deviceInfo->maxOutputChannels
              };
          }
          deviceList[i] = &outputDevices[deviceName];
      }
  }

  // TODO: move ids setter into separate function
  if (ids.output == -1) ids.output = Pa_GetDefaultOutputDevice();
  loopbackAvailable = deviceList[ids.output]->ids.find("Loopback") == deviceList[ids.output]->ids.end() ? false : true;
  apiName = loopbackAvailable ? "Windows WASAPI" : "MME";
  ids.output = deviceList[ids.output]->ids[apiName];

  if (ids.input == -1) ids.input = deviceList[Pa_GetDefaultInputDevice()]->ids[apiName];
  if (ids.streamOut == -1) ids.streamOut = deviceList[Pa_GetDefaultOutputDevice()]->ids[apiName];
  if (ids.output == -1) ids.output = deviceList[Pa_GetDefaultOutputDevice()]->ids[apiName];

  // set the default virtual input device - assumes the user is using
  // VB-Audio's Virtual Cable https://vb-audio.com/Cable/
  ids.vInput = outputDevices["CABLE Input (VB-Audio Virtual Cable)"].ids[apiName];
  ids.vOutput = inputDevices["CABLE Output (VB-Audio Virtual Cable)"].ids[apiName];

  // TODO: move sample rate checks into separate function
  if (Pa_GetDeviceInfo(ids.vInput)->defaultSampleRate != 48000) {
      MessageBox(NULL, L"Sample rate of VB-Audio Cable Input is not set to 48000Hz.", L"Error", MB_ICONERROR | MB_OK);
      log(CRITICAL, "Sample rate of VB-Audio Cable Input is not set to 48000Hz. Exiting.");
      exit(1);
      return;
  }

  if (Pa_GetDeviceInfo(ids.input)->defaultSampleRate != 48000) {
      MessageBox(NULL, L"Sample rate of default input device is not set to 48000Hz.", L"Error", MB_ICONERROR | MB_OK);
      log(CRITICAL, "Sample rate of default input device is not set to 48000Hz. Exiting.");
      exit(1);
      return;
  }

  if (Pa_GetDeviceInfo(ids.vOutput)->defaultSampleRate != 48000) {
      log(WARN, "Sample rate of VB-Audio Cable Input is not set to 48000Hz.");
      MessageBox(NULL, L"Sample rate of VB-Audio Cable Output is not set to 48000Hz and may result in unexpected behaviour.", L"Warning", MB_ICONWARNING | MB_OK);
  }


  log(INFO, "Device settings retrieved");
  log(INFO, "Input Device: " + std::string(Pa_GetDeviceInfo(ids.input)->name));
  log(INFO, "Stream Output Device: " + std::string(Pa_GetDeviceInfo(ids.streamOut)->name));
  log(INFO, "Output Device: " + std::string(Pa_GetDeviceInfo(ids.output)->name));
}

int MainInterface::GetChannels(int id, bool isInput) {
  PaStreamParameters params;
  params.hostApiSpecificStreamInfo = NULL;
  params.sampleFormat = PA_SAMPLE_TYPE;
  params.device = id;
  params.hostApiSpecificStreamInfo = NULL;
  params.sampleFormat = PA_SAMPLE_TYPE;
  params.channelCount = 2;
  params.suggestedLatency = Pa_GetDeviceInfo(id)->defaultLowInputLatency;

  PaError err = Pa_IsFormatSupported(isInput ? &params : NULL,
                                     isInput ? NULL : &params, 48000);
  return err != 0 ? 1 : 2;
}

/* Reset functions for portaudio objects, called when corresponding devices are
 * changed in the GUI */
void MainInterface::Reset(bool devicesChanged) {
  ResetStreams();
  fxManager->SetFramesPerBuffer(settings["framesPerBuffer"].get<int>());

  Pa_Terminate();
  Pa_Initialize();
  log(INFO, "PortAudio restarted");

  GetDeviceSettings();

  SetupStreams();

  WaitForReady();

  if (!devicesChanged) {
    SaveSettings();
    log(INFO, "Settings saved");
  }
}

void MainInterface::ResetStreams() {
  soundboardManager->ResetStreams();
  fxManager->ResetStreams();
  noiseGen.reset();
  monitor.reset();
  player.reset();
  passthrough.reset();
  cleanOutput.reset();
  log(INFO, "Streams reset");
}

void MainInterface::SetupStreams() {
  inputQueue = new std::queue<float>();
  playbackQueue = new std::queue<float>();
  //int loopbackdevice = GetCorrespondingLoopbackDevice(ids.streamOut);
  int loopbackDevice = loopbackAvailable ? deviceList[ids.output]->ids["Loopback"] : -1;
  int loopbackChannels = loopbackDevice >= 0 ? deviceList[loopbackDevice]->nChannels : -1;

  noiseGen.reset(new NoiseGenerator(ids.streamOut, deviceList[ids.streamOut]->nChannels,
                                    settings["sampleRate"].get<int>()));
  monitor.reset(new Monitor(
      ids.input, deviceList[ids.input]->nChannels,
      ids.output, deviceList[ids.output]->nChannels,
      settings["sampleRate"].get<int>(), settings["framesPerBuffer"].get<int>(),
      rootDir, inputQueue, playbackQueue));
  player.reset(new Player(
      loopbackDevice, loopbackChannels,
      ids.vInput, deviceList[ids.vInput]->nChannels,
      settings["sampleRate"].get<int>(), settings["framesPerBuffer"].get<int>(),
      rootDir, playbackQueue));
  passthrough.reset(new Passthrough(
      ids.input, deviceList[ids.input]->nChannels,
      ids.vInput, deviceList[ids.vInput]->nChannels,
      settings["sampleRate"].get<int>(), settings["framesPerBuffer"].get<int>(),
      rootDir, inputQueue));
  if (ids.streamOut != ids.output)
    cleanOutput.reset(new CleanOutput(loopbackDevice, loopbackChannels,
                                      ids.output, deviceList[ids.output]->nChannels,
                                      settings["sampleRate"].get<int>(), 1));
  else
    cleanOutput.reset();

  soundboardManager->SetStreams(passthrough, monitor, player);
  fxManager->SetStreams(passthrough, monitor);
}
