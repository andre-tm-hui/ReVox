#include "loggableobject.h"

LoggableObject::LoggableObject(std::string objType, std::string loggerName)
    : objType(objType), loggerName(loggerName) {
  // logger = spdlog::get(loggerName);
}

LoggableObject::~LoggableObject() { spdlog::drop_all(); }

int LoggableObject::log(LogLevel level, std::string msg) {
  if (logger == nullptr) {
    logger = spdlog::get(loggerName);
  }
  if (logger == nullptr) return 2;
  try {
    switch (level) {
      case INFO:
        logger->info("[{}] {}", objType, msg);
        break;
      case ERR:
        logger->error("[{}] {}", objType, msg);
        break;
      case WARN:
        logger->warn("[{}] {}", objType, msg);
        break;
      case CRITICAL:
        logger->critical("[{}] {}", objType, msg);
        break;
      case DEBUG:
        logger->debug("[{}] {}", objType, msg);
        break;
      default:
        break;
    }
    return 0;
  } catch (const spdlog::spdlog_ex& ex) {
    return 1;
  }
}
