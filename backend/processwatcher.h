#ifndef PROCESSWATCHER_H
#define PROCESSWATCHER_H

#include <Windows.h>

#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_set>

#include "../util/loggableobject.h"
#include "Psapi.h"

#define CHECK_AFTER 200

class ProcessWatcher : LoggableObject {
 public:
  ProcessWatcher();

  void start();
  void stop();
  void loop();

  void setCallbacks(std::function<void()> registerHook,
                    std::function<void()> unregisterHook) {
    this->registerHook = registerHook;
    this->unregisterHook = unregisterHook;
  }

 private:
  std::string getName(DWORD processId);
  std::function<void()> registerHook, unregisterHook;

  // EasyAntiCheat games won't work since they block the dll from loading, but
  // the games seem to run fine regardless. Valorant won't run well if the hook
  // isn't disabled.
  std::unordered_set<std::string> runningBlacklistedProcesses,
      blacklistedProcesses = {"RiotClientCrashHandler.exe",
                              "RiotClientServices.exe", "VALORANT.exe",
                              "UnrealCEFSubProcess.exe",
                              "VALORANT-Win64-Shipping.exe"};
  std::unordered_set<DWORD> runningProcesses;
  std::atomic<bool> running = false;

  DWORD aProcesses[1024], cbNeeded, cProcesses;
  int i;
  bool blocking = false;
};

#endif  // PROCESSWATCHER_H
