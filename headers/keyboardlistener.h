#ifndef KEYBOARDLISTENER_H
#define KEYBOARDLISTENER_H

#include "audiomanager.h"
#include <Windows.h>
#include <hidsdi.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <devpkey.h>

class KeyboardListener
{
public:
    KeyboardListener();

    LRESULT static CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    int Start(HWND hWnd = NULL);

    RAWINPUTDEVICE rid;

    static AudioManager* audioManager;
};



#endif // KEYBOARDLISTENER_H
