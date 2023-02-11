#ifndef BASEINTERFACE_H
#define BASEINTERFACE_H

#include <string>
#include <filesystem>
#include <fstream>
#include "../nlohmann/json.hpp"

using namespace nlohmann;

class BaseInterface
{
public:
    BaseInterface(std::string rootDir);
    virtual ~BaseInterface() {};

    template <typename T>
    void UpdateSettings(std::string path, T val) {
        json *obj = &settings;

        while (path.find('/') != path.npos) {
            obj = &((*obj)[path.substr(0, path.find('/'))]);
            path = path.substr(path.find('/') + 1);
        }
        (*obj)[path] = val;

        SaveSettings();
    }

    json GetSetting(std::string path);
    std::string GetRootDir() { return rootDir; }

protected:
    virtual bool LoadSettings();
    void SaveSettings();

    std::string rootDir = "";
    std::string dataPath = "";
    json settings = NULL;
    //sjson defaultObj = R"({})"_json;
};

#endif // BASEINTERFACE_H
