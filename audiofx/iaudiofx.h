#ifndef IAUDIOFX_H
#define IAUDIOFX_H

#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "../util/loggableobject.h"

struct paramType {
  std::string type;
  std::string unit;
  std::string toolTip;
  int min;
  int max;
};

class IAudioFX : public LoggableObject {
 public:
  IAudioFX(std::string objType) : LoggableObject(objType){};
  virtual void Process(float *buf) {}
  virtual void Reset(int framesPerBuffer = -1) {}
  virtual float Get(std::string item) { return 0; }
  virtual void Set(std::string item, int val) {}
  virtual bool GetEnabled() { return false; }
  virtual void SetEnabled(bool enabled) {}
  std::unordered_map<std::string, paramType> GetFXWidgets() { return paramMap; }

 protected:
  std::unordered_map<std::string, paramType> paramMap;
  int framesPerBuffer;
};

#endif  // IAUDIOFX_H
