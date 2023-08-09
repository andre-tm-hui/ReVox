#ifndef MAININTERFACE_H
#define MAININTERFACE_H

#ifndef VER_NO
#define VER_NO "0.4"
#endif

#ifndef ROOTNAME
#if _DEBUG
#define ROOTNAME "/ReVoxDebug/"
#else
#define ROOTNAME "/ReVox/"
#endif
#endif

#include <functional>

#include "../backend/streams/cleanoutput.h"
#include "../backend/streams/noisegenerator.h"
#include "../util/loggableobject.h"
#include "fxmanager.h"
#include "soundboardmanager.h"
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

struct deviceIDs {
  int input;
  int output;
  int streamOut;
  int vInput;
  int vOutput;
};

class MainInterface : public BaseInterface, public LoggableObject {
 public:
  MainInterface();
  ~MainInterface() override;

  bool KeyEvent(int keycode, std::string deviceName, int event);
  void Reset(bool devicesChanged = false);

  int GetCurrentOutputDevice() { return ids.output; }
  void SetCurrentOutputDevice(int id);

  std::shared_ptr<SoundboardManager> GetSoundboardManager() {
    return soundboardManager;
  }
  std::shared_ptr<FXManager> GetFXManager() { return fxManager; }

  std::map<int, device*> GetDeviceList() { return deviceList; }
  std::map<std::string, device>& GetInputDevices() { return inputDevices; }
  std::map<std::string, device>& GetOutputDevices() { return outputDevices; }
  std::map<std::string, device>& GetLoopbackDevices() { return loopbackDevices; }

  void SetBlocked(bool blocked) { this->blocked = blocked; }
  int GetLastKeycode() { return lastKeycode; }
  void SetLastKeycode(int keycode) { lastKeycode = keycode; }
  std::function<bool(bool)> ToggleInputBlocking;

  bool IsLoopbackAvailable() { return loopbackAvailable; }

 private:
  void SetupStreams();
  void ResetStreams();
  void GetDeviceSettings();
  int GetChannels(int id, bool isInput);
  void WaitForReady();

  bool blocked = false,
      loopbackAvailable = false;
  std::string apiName = "";
  int lastKeycode = -1;
  std::queue<float> *inputQueue, *playbackQueue;

  std::shared_ptr<SoundboardManager> soundboardManager;
  std::shared_ptr<FXManager> fxManager;

  std::shared_ptr<Passthrough> passthrough;
  std::shared_ptr<Player> player;
  std::shared_ptr<Monitor> monitor;
  std::shared_ptr<CleanOutput> cleanOutput;
  std::shared_ptr<NoiseGenerator> noiseGen;

  deviceIDs ids = {-1, -1, -1, -1};
  std::map<int, device*> deviceList;
  std::map<std::string, device> inputDevices;
  std::map<std::string, device> outputDevices;
  std::map<std::string, device> loopbackDevices;

  //TODO: add "Use Default Device" to settings, add input/microphone
  json defaultSettings = R"(
        {
            "outputDevice": "No Device Detected",
            "sampleRate": 48000,
            "framesPerBuffer": 2048,
            "hudPosition": -1,
            "startWithWindows": false,
            "autocheckUpdates": false,
            "firstTime": true,
            "blockInputs": false,
            "detectKeyboard": true
        })"_json;
};

#endif  // MAININTERFACE_H
