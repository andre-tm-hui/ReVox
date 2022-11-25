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

//template <typename T> {};

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
    //void SetBind(RebindData data, std::string path, bool isBasic);
    void RemoveBind(int idx);



    //json GetSetting(std::string path);
    json GetDefaultHotkey() { return defaultObj; }

    bool KeybindExists(int keycode, std::string deviceName);

    //std::string GetRootDir() { return rootDir; };

protected:
    bool LoadSettings() override;
    //void SaveSettings();

    //std::string rootDir = "";
    //std::string dataPath = "";
    //json settings = NULL;
    json defaultObj = R"({})"_json;

private:
    virtual void StartEvent(std::string path, int event) {}
    json* GetObj(std::string path);
    std::string FindFromKeycode(int keycode, std::string deviceName);

    RebindData rebindData;
    json* rebindItem = nullptr;
};

#endif // BASEMANAGER_H
