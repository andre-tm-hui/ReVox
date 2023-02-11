#ifndef BASEMANAGER_H
#define BASEMANAGER_H

#include <filesystem>
#include <string>
#include <fstream>
#include <ranges>
#include "../nlohmann/json.hpp"
#include "baseinterface.h"

using namespace nlohmann;

struct RebindData {
    std::string deviceName;
    int keycode;
};

class BaseManager : public BaseInterface
{
public:
    BaseManager(std::string rootDir);

    void KeyEvent(int keycode, std::string deviceName, int event);

    virtual void SetMonitoringVol(int n) {}
    int GetMonitoringVol();

    void StartRebinding(std::string path);
    bool IsRebinding();
    RebindData GetRebindData();
    void RemoveBind(int idx);

    json GetDefaultHotkey() { return defaultObj; }

    bool KeybindExists(int keycode, std::string deviceName);

protected:
    bool LoadSettings() override;

    json defaultObj = R"({})"_json;

private:
    virtual void StartEvent(std::string path, int event) {}
    json* GetObj(std::string path);
    std::string FindFromKeycode(int keycode, std::string deviceName);

    RebindData rebindData;
    json* rebindItem = nullptr;
};

#endif // BASEMANAGER_H
