#ifndef LOGGABLEOBJECT_H
#define LOGGABLEOBJECT_H

#include "spdlog/spdlog.h"

enum LogLevel {
  INFO,
  ERR,
  WARN,
  CRITICAL,
  DEBUG,
};

class LoggableObject {
 public:
  LoggableObject(std::string objType = "", std::string loggerName = "logger");
  ~LoggableObject();
  int log(LogLevel level, std::string msg);

 protected:
  std::shared_ptr<spdlog::logger> logger;

 private:
  std::string objType, loggerName;
};

#endif  // ILOGGABLEOBJECT_H
