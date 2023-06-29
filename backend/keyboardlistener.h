#ifndef KEYBOARDLISTENER_H
#define KEYBOARDLISTENER_H

#include <Windows.h>

#include "../interface/maininterface.h"
#include "processwatcher.h"

UINT const WM_HOOK = WM_APP + 1;
UINT const WM_INSTALLHOOK = WM_HOOK + 1;
UINT const WM_UNINSTALLHOOK = WM_INSTALLHOOK + 1;

typedef bool(WINAPI* PIH)(HWND);
typedef bool(WINAPI* PUH)();

struct DecisionRecord {
  USHORT virtualKeyCode;
  BOOL decision;

  DecisionRecord(USHORT _virtualKeyCode, BOOL _decision)
      : virtualKeyCode(_virtualKeyCode), decision(_decision) {}
};

class KeyboardListener : public LoggableObject {
 public:
  KeyboardListener();

  LRESULT static CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam);
  static DWORD WINAPI MessageThreadProc(LPVOID lpParameter);

  int Start(HWND hWnd = NULL);
  void EnableBlocking();
  void DisableBlocking();
  bool ToggleInputBlocking(bool enabled);

  RAWINPUTDEVICE rid;
  HHOOK hhookKbdListener;

  static MainInterface* mi;
  static HWND hWnd;
  static std::deque<DecisionRecord> decisionBuffer;
  static DecisionRecord decision;

 private:
  void RegisterHook();
  void UnregisterHook();

  PIH InstallHook;
  PUH UninstallHook;

  ProcessWatcher p;

  static const LPCWSTR blockingDllName;
  static HINSTANCE blockingDllLib;
};

#endif  // KEYBOARDLISTENER_H
