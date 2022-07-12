#ifndef KEYBOARDLISTENER_H
#define KEYBOARDLISTENER_H

#include "audiomanager.h"
#include <condition_variable>
#include <mutex>
#include <Windows.h>

class KeyboardListener
{
public:
    KeyboardListener();

    ~KeyboardListener();

    LRESULT static CALLBACK KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam);

    int Start();

    HHOOK hhookKbdListener;

    static AudioManager* audioManager;

    static bool rebinding;
    static bool ready() { return !rebinding; }

    static int rebindTo;

    static std::mutex* mtx;
    static std::condition_variable* cv;
};



#endif // KEYBOARDLISTENER_H
