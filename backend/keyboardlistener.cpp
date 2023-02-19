#include "keyboardlistener.h"

std::deque<DecisionRecord> KeyboardListener::decisionBuffer({});
HWND KeyboardListener::hWnd = 0;

KeyboardListener::KeyboardListener() : LoggableObject("KeyboardListener") {}

LRESULT CALLBACK KeyboardListener::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                           LPARAM lParam) {
  switch (uMsg) {
    case WM_INPUT: {
      bool block = false;
      // Get the RawInputData
      UINT dwSize;
      GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize,
                      sizeof(RAWINPUTHEADER));
      LPBYTE lpb = new BYTE[dwSize];
      if (lpb == NULL) return 0;
      GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize,
                      sizeof(RAWINPUTHEADER));

      RAWINPUT* raw = (RAWINPUT*)lpb;

      // Get the identifying data - code, deviceName, and the action - event
      USHORT code = raw->data.keyboard.VKey;
      USHORT event = raw->data.keyboard.Flags;

      WCHAR ridDeviceName[256];
      UINT cbDataSize = 256;
      GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, ridDeviceName,
                            &cbDataSize);

      std::wstring ws(ridDeviceName);
      block = mi->KeyEvent(code, std::string(ws.begin(), ws.end()), event);
      decisionBuffer.push_back(DecisionRecord(code, block));
      break;
    }

      // Credits to Vít Blecha, check out the blog post:
      // https://www.codeproject.com/Articles/716591/Combining-Raw-Input-and-keyboard-Hook-to-selective
    case WM_HOOK: {
      USHORT virtualKeyCode = (USHORT)wParam;
      USHORT keyPressed = lParam & 0x80000000 ? 0 : 1;
      WCHAR text[128];

      // Check the buffer if this Hook message is supposed to be blocked
      // return 1 if it is
      BOOL blockThisHook = FALSE;
      BOOL recordFound = FALSE;
      int index = 1;
      if (!decisionBuffer.empty()) {
        // Search the buffer for the matching record
        std::deque<DecisionRecord>::iterator iterator = decisionBuffer.begin();
        while (iterator != decisionBuffer.end()) {
          if (iterator->virtualKeyCode == virtualKeyCode) {
            blockThisHook = iterator->decision;
            recordFound = TRUE;
            // Remove this and all preceding messages from the buffer
            for (int i = 0; i < index; ++i) {
              decisionBuffer.pop_front();
            }
            // Stop looking
            break;
          }
          ++iterator;
          ++index;
        }
      }

      // Apply the decision
      if (blockThisHook) {
        return 1;
      }
      return 0;
    }
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int KeyboardListener::Start(HWND hWnd) {
  HINSTANCE hInstance = GetModuleHandle(NULL);
  WNDCLASSEXW wcex = {};
  wcex.cbSize = sizeof(wcex);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.lpszClassName = TEXT("KbdListener");
  wcex.hInstance = hInstance;
  if (RegisterClassEx(&wcex))
    hWnd = CreateWindowEx(0, wcex.lpszClassName, L"KbdListener", 0, 0, 0, 0, 0,
                          HWND_MESSAGE, NULL, hInstance, NULL);
  this->hWnd = hWnd;

  rid.usUsagePage = 0x01;
  rid.usUsage = 0x06;
  rid.dwFlags = RIDEV_INPUTSINK;
  rid.hwndTarget = hWnd;
  if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
    std::cout << "failed" << std::endl;
  }
  SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)&WndProc);

  p.setCallbacks(std::bind(&KeyboardListener::RegisterHook, this),
                 std::bind(&KeyboardListener::UnregisterHook, this));

  mi->ToggleInputBlocking = std::bind(&KeyboardListener::ToggleInputBlocking,
                                      this, std::placeholders::_1);

  blockingDllLib = LoadLibraryW(blockingDllName);
  if (blockingDllLib) {
    log(INFO, "Input blocking available");
    InstallHook = (PIH)GetProcAddress(blockingDllLib, "InstallHook");
    UninstallHook = (PUH)GetProcAddress(blockingDllLib, "UninstallHook");
    if (InstallHook == NULL) log(WARN, "InstallHook not found");
    if (UninstallHook == NULL) log(WARN, "UninstallHook not found");
  }

  ToggleInputBlocking(mi->GetSetting("blockInputs").get<bool>());

  return 0;
}

void KeyboardListener::EnableBlocking() {
  RegisterHook();
  p.start();
}

void KeyboardListener::DisableBlocking() {
  UnregisterHook();
  p.stop();
}

bool KeyboardListener::ToggleInputBlocking(bool enabled) {
  if (!blockingDllLib) {
    log(WARN, "Input blocking not available - requires InputBlockingDLL.dll");
    return false;
  }

  if (enabled)
    EnableBlocking();
  else
    DisableBlocking();
  return true;
}

void KeyboardListener::RegisterHook() {
  InstallHook(hWnd);
  log(INFO, "Hook registered");
}

void KeyboardListener::UnregisterHook() {
  UninstallHook();
  log(INFO, "Hook unregistered");
}
