#include "audiofx/reverberator.h"

Reverberator::Reverberator(int bufSize) : IAudioFX(), bufSize(bufSize)
{
    rev.reset(new revmodel());
    paramMap["Damp"] = {"FXDial", "%", "Set how quickly the echoes decay", 0, 100};
    paramMap["Wet"] = {"FXDial", "%", "Set the volume of the reverb", 0, 100};
    paramMap["Dry"] = {"FXDial", "%", "Set the volume of your voice", 0, 100};
    paramMap["Roomsize"] = {"FXDial", "%", "Set the size of the room", 0, 100};
    paramMap["Width"] = {"FXDial", "%", "Set the width of the room", 0, 100};
}

void Reverberator::Process(float *buf) {
    rev->processmono(buf, buf, bufSize, 1);
}

void Reverberator::Reset(int framesPerBuffer) {
    if (framesPerBuffer > 0) this->bufSize = framesPerBuffer;
    rev->mute();
}

float Reverberator::Get(std::string item) {
    if (item == "Damp")
        return rev->getdamp();
    else if (item == "Wet")
        return rev->getwet();
    else if (item == "Dry")
        return rev->getdry();
    else if (item == "Roomsize")
        return rev->getroomsize();
    else if (item == "Width")
        return rev->getwidth();

    return 0.f;
}

void Reverberator::Set(std::string item, int val) {
    if (item == "Damp")
        rev->setdamp(val / 100.f);
    else if (item == "Wet")
        rev->setwet(val / 100.f);
    else if (item == "Dry")
        rev->setdry(val / 100.f);
    else if (item == "Roomsize")
        rev->setroomsize(val / 100.f);
    else if (item == "Width")
        rev->setwidth(val / 100.f);
}

bool Reverberator::GetEnabled() {
    return rev->getEnabled();
}

void Reverberator::SetEnabled(bool enabled) {
    rev->setEnabled(enabled);
}
