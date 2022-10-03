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
            if (audioManager->soundboardHotkeys.find(std::to_string(p->vkCode)) == audioManager->soundboardHotkeys.end() &&
                    audioManager->voiceFXHotkeys.find(std::to_string(p->vkCode)) == audioManager->voiceFXHotkeys.end())
            {
                rebindTo = p->vkCode;
                rebinding = false;
                cv->notify_all();
            }
        }
        else
        {
            json bindSettings;
            bool isSoundboard = true;
            if (audioManager->soundboardHotkeys.find(std::to_string(p->vkCode)) != audioManager->soundboardHotkeys.end())
            {
                bindSettings = audioManager->soundboardHotkeys[std::to_string(p->vkCode)];
            }
            else if (audioManager->voiceFXHotkeys.find(std::to_string(p->vkCode)) != audioManager->voiceFXHotkeys.end())
            {
                bindSettings = audioManager->voiceFXHotkeys[std::to_string(p->vkCode)];
                isSoundboard = false;
            }
            if (!bindSettings.empty()) {
                switch (wParam) {
                case WM_KEYDOWN:
                    if (isSoundboard)
                    {
                        if (!audioManager->recording) {
                            audioManager->Play(p->vkCode);
                        }
                    }
                    else
                    {
                        if (audioManager->passthrough != nullptr)
                        {
                            audioManager->passthrough->SetFX(audioManager->voiceFXHotkeys[std::to_string(p->vkCode)]);
                            *(audioManager->fxHotkey) = p->vkCode;
                        }
                    }

                    break;
                case WM_KEYUP:
                    if (isSoundboard)
                    {
                        // stop recording when button is released
                        if (audioManager->recording)
                        {
                            audioManager->StopRecording();
                        }
                    }
                    break;
                case WM_SYSKEYDOWN:
                    if (isSoundboard)
                    {
                        // secondary record button - used to record over already existing clips
                        if (!audioManager->recording)
                        {
                            audioManager->Record(p->vkCode);
                        }
                    }
                    break;
                case WM_SYSKEYUP:
                    if (isSoundboard)
                    {
                        // stop recording when button is released
                        if (audioManager->recording)
                        {
                            audioManager->StopRecording();
                        }
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

