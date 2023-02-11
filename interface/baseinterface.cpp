#include "baseinterface.h"

BaseInterface::BaseInterface(std::string rootDir) : rootDir(rootDir)
{

}

json BaseInterface::GetSetting(std::string path) {
    json obj = settings;

    while (path.find('/') != path.npos) {
        obj = obj[path.substr(0, path.find('/'))];
        path = path.substr(path.find('/') + 1);
    }
    if (obj[path].is_null()) {
        obj[path] = 0;
        SaveSettings();
    }

    return obj[path];
}

bool BaseInterface::LoadSettings() {
    std::ifstream settingsFile(rootDir + dataPath);
    if (!settingsFile.fail()) {
        json temp;
        settingsFile >> temp;
        if (temp.find("monitorMic") != temp.end()) {
            SaveSettings();
            return false;
        }

        for (auto& [id,obj] : settings.items())
            if (temp.find(id) == temp.end())
                temp[id] = obj;

        settings = temp;
        return true;
    }
    SaveSettings();
    //SetMonitoringVol(settings["monitor"]);
    return false;
}

void BaseInterface::SaveSettings() {
    std::ofstream settingsFile(rootDir + dataPath);
    if (settingsFile.fail()) return;
    settingsFile << std::setw(4) << settings << '\n';
}

