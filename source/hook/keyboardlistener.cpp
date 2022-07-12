#include "keyboardlistener.h"

KeyboardListener::KeyboardListener()
{

}

// define some static properties
int KeyboardListener::rebindTo = -1;
bool KeyboardListener::rebinding = false;
std::mutex* KeyboardListener::mtx = new std::mutex();
std::condition_variable* KeyboardListener::cv = new std::condition_variable();


LRESULT CALLBACK KeyboardListener::KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
        if (rebinding)
        {
            if (audioManager->keybinds.find(p->vkCode) == audioManager->keybinds.end())
            {
                rebindTo = p->vkCode;
                rebinding = false;
                cv->notify_all();
            }
        }
        else
        {
            if (audioManager->keybinds.find(p->vkCode) != audioManager->keybinds.end()) {
                keybind bindSettings = audioManager->keybinds[p->vkCode];

                switch (wParam) {
                case WM_KEYDOWN:
                    if (!audioManager->recording) {
                        // check if button has a recorded file, otherwise, start recording
                        if (audioManager->player->CanPlay(p->vkCode))
                        {
                            audioManager->player->Play(p->vkCode);
                            audioManager->monitor->Play(p->vkCode);
                        }
                        // if not, start recording
                        else
                        {
                            if (bindSettings.recordInput) audioManager->inputDeviceRecorder->Record(p->vkCode);
                            if (bindSettings.recordLoopback) audioManager->loopbackRecorder->Record(p->vkCode, bindSettings.padAudio, !bindSettings.recordInput);
                            audioManager->recording = true;
                        }
                    }
                    break;
                case WM_KEYUP:
                    // stop recording when button is released
                    if (audioManager->recording)
                    {
                        if (bindSettings.recordInput) audioManager->inputDeviceRecorder->Stop(p->vkCode);
                        if (bindSettings.recordLoopback)
                        {
                            audioManager->loopbackRecorder->Stop(p->vkCode);
                            audioManager->loopbackRecorder->Merge(p->vkCode);
                        }
                        audioManager->recording = false;
                    }
                    break;
                case WM_SYSKEYDOWN:
                    // secondary record button - used to record over already existing clips
                    if (!audioManager->recording)
                    {
                        if (bindSettings.recordInput) audioManager->inputDeviceRecorder->Record(p->vkCode);
                        if (bindSettings.recordLoopback) audioManager->loopbackRecorder->Record(p->vkCode, bindSettings.padAudio, !bindSettings.recordInput);
                        audioManager->recording = true;
                    }
                    break;
                case WM_SYSKEYUP:
                    // stop recording when button is released
                    if (audioManager->recording)
                    {
                        if (bindSettings.recordInput) audioManager->inputDeviceRecorder->Stop(p->vkCode);
                        if (bindSettings.recordLoopback)
                        {
                            audioManager->loopbackRecorder->Stop(p->vkCode);
                            audioManager->loopbackRecorder->Merge(p->vkCode);
                        }
                        audioManager->recording = false;
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
    return CallNextHookEx(0, nCode, wParam, lParam);
}

int KeyboardListener::Start()
{
    hhookKbdListener = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardEvent, 0, 0);
    return 0;
}


KeyboardListener::~KeyboardListener()
{
    UnhookWindowsHookEx(hhookKbdListener);
}

