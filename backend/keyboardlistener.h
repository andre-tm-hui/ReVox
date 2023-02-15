#ifndef KEYBOARDLISTENER_H
#define KEYBOARDLISTENER_H

#include <Windows.h>
#include <cfgmgr32.h>
#include <hidsdi.h>
#include <initguid.h>

#include "../interface/maininterface.h"
#include "devpkey.h"

class KeyboardListener {
 public:
  KeyboardListener();

  LRESULT static CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam);

  int Start(HWND hWnd = NULL);

  RAWINPUTDEVICE rid;

  static MainInterface* mi;
};

#endif  // KEYBOARDLISTENER_H
