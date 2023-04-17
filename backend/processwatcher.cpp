#include "processwatcher.h"

ProcessWatcher::ProcessWatcher() : LoggableObject("ProcessWatcher") {}

void ProcessWatcher::start() {
  running = true;
  std::thread t(&ProcessWatcher::loop, this);
  t.detach();
}

void ProcessWatcher::stop() { running = false; }

void ProcessWatcher::loop() {
  while (running) {
    runningBlacklistedProcesses = {};
    if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
      cProcesses = cbNeeded / sizeof(DWORD);

      for (i = 0; i < cProcesses; i++) {
        if (aProcesses[i] != 0) {
          std::string processName = getName(aProcesses[i]);
          if (blacklistedProcesses.find(processName) !=
              blacklistedProcesses.end()) {
            runningBlacklistedProcesses.insert(processName);
            log(INFO, "Blacklisted process launched");
          }
        }
      }
    }
    if (!runningBlacklistedProcesses.empty() && !blocking) {
      unregisterHook();
      blocking = true;
    } else if (runningBlacklistedProcesses.empty() && blocking) {
      registerHook();
      blocking = false;
    }
    std::this_thread::sleep_for(
        std::chrono::duration(std::chrono::milliseconds(CHECK_AFTER)));
  }
}

std::string ProcessWatcher::getName(DWORD processId) {
  WCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                FALSE, aProcesses[i]);
  if (hProcess != NULL) {
    HMODULE hMod;
    DWORD cbn;
    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbn)) {
      GetModuleBaseName(hProcess, hMod, szProcessName,
                        sizeof(szProcessName) / sizeof(WCHAR));
    }
  }
  std::wstring wName(szProcessName);
  std::string dName(wName.begin(), wName.end());
  return dName;
}
