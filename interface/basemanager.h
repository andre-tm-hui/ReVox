#ifndef BASEMANAGER_H
#define BASEMANAGER_H

#include <filesystem>
#include <fstream>
#include <ranges>
#include <string>

#include "../nlohmann/json.hpp"
#include "../util/loggableobject.h"
#include "baseinterface.h"

using namespace nlohmann;

struct RebindData {
  std::string deviceName;
  int keycode;
};

class BaseManager : public BaseInterface, public LoggableObject {
 public:
  BaseManager(std::string rootDir, std::string objType);

  void KeyEvent(int keycode, std::string deviceName, int event);

  virtual void SetMonitoringVol(int n) {}
  int GetMonitoringVol() { return settings["monitor"].get<float>() * 100; }

  void StartRebinding(std::string path) { rebindItem = GetObj(path); }
  bool IsRebinding() { return rebindItem != nullptr; }
  RebindData GetRebindData() { return rebindData; }
  void RemoveBind(int idx);

  json GetDefaultHotkey() { return defaultObj; }

  bool KeybindExists(int keycode, std::string deviceName,
                     bool ignoreDeviceName) {
    return FindFromKeycode(keycode, deviceName, ignoreDeviceName).empty()
               ? false
               : true;
  }

 protected:
  bool LoadSettings() override;

  json defaultObj = R"({})"_json;

 private:
  virtual void StartEvent(std::string path, int event) {}
  json* GetObj(std::string path);
  std::string FindFromKeycode(int keycode, std::string deviceName,
                              bool ignoreDeviceName = true);

  RebindData rebindData;
  json* rebindItem = nullptr;
};

#endif  // BASEMANAGER_H
