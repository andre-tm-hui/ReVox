#include "basemanager.h"

BaseManager::BaseManager(std::string rootDir, std::string objType)
    : BaseInterface(rootDir), LoggableObject(objType) {}

void BaseManager::KeyEvent(int keycode, std::string deviceName, int event) {
  log(INFO, "Input recognized from " + deviceName);
  if (rebindItem != nullptr) {
    switch (keycode) {
      case 8:  // backspace to clear
        (*rebindItem)["keycode"] = -1;
        (*rebindItem)["deviceName"] = "";
        break;
      case 27:  // esc to cancel
        break;
      default:
        (*rebindItem)["keycode"] = keycode;
        (*rebindItem)["deviceName"] = deviceName;
    }

    rebindData = {(*rebindItem)["deviceName"], (*rebindItem)["keycode"]};
    rebindItem = nullptr;
    SaveSettings();
  } else {
    std::string path = FindFromKeycode(keycode, deviceName);
    if (path.size() > 0) StartEvent(path, event);
  }
}

void BaseManager::RemoveBind(int idx) {
  log(INFO, "Removing bind " + std::to_string(idx));
  json* obj = &settings["hotkeys"];
  if (std::filesystem::exists(rootDir + "samples/" + std::to_string(idx) +
                              ".mp4"))
    std::filesystem::remove(rootDir + "samples/" + std::to_string(idx) +
                            ".mp4");
  int i;
  for (i = 0; i < obj->size(); i++) {
    if (i > idx) {
      (*obj)[std::to_string(i - 1)] = (*obj)[std::to_string(i)];
      if (std::filesystem::exists(rootDir + "samples/" + std::to_string(i) +
                                  ".mp4"))
        std::filesystem::rename(
            rootDir + "samples/" + std::to_string(i) + ".mp4",
            rootDir + "samples/" + std::to_string(i - 1) + ".mp4");
    }
  }
  obj->erase(std::to_string(i - 1));
  SaveSettings();
  log(INFO, "Removed bind " + std::to_string(idx));
}

std::string BaseManager::FindFromKeycode(int keycode, std::string deviceName,
                                         bool ignoreDeviceName) {
  for (auto const& [id, obj] : settings.items()) {
    if (id != "hotkeys" && obj.type() == json::value_t::object &&
        obj.find("keycode") != obj.end()) {
      if (obj["keycode"] == keycode &&
          (ignoreDeviceName ? true : obj["deviceName"] == deviceName))
        return id;
    }
  }
  for (auto const& [idx, obj] : settings["hotkeys"].items()) {
    if (obj["keycode"] == keycode &&
        (ignoreDeviceName ? true : obj["deviceName"] == deviceName))
      return "hotkeys/" + idx;
  }

  return "";
}

json* BaseManager::GetObj(std::string path) {
  bool isBasic = true;
  json* obj = &settings;

  while (path.find('/') != path.npos) {
    std::string key = path.substr(0, path.find('/'));
    if (key == "hotkeys") isBasic = false;
    obj = &((*obj)[key]);
    path = path.substr(path.find('/') + 1);
  }
  if (obj->find(path) == obj->end())
    (*obj)[path] =
        isBasic ? R"({"keycode":-1, "deviceName":""})"_json : defaultObj;
  obj = &((*obj)[path]);
  return obj;
}

bool BaseManager::LoadSettings() {
  std::ifstream settingsFile(rootDir + dataPath);
  if (!settingsFile.fail()) {
    json temp;
    settingsFile >> temp;
    if (temp.find("monitor") == temp.end()) {
      SaveSettings();
      return false;
    }

    for (auto& [id, obj] : settings.items())
      if (temp.find(id) == temp.end()) temp[id] = obj;
    if (temp.find("hotkeys") != temp.end())
      for (auto& [idx, keybind] : temp["hotkeys"].items())
        for (auto const& [k, v] : defaultObj.items())
          if (keybind.find(k) == keybind.end()) keybind[k] = v;

    settings = temp;
    log(INFO, "Settings found and loaded");
    return true;
  }
  SaveSettings();
  log(INFO, "Settings created and saved");
  return false;
}
