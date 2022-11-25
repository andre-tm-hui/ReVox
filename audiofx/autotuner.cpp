#include "audiofx/autotuner.h"

Autotuner::Autotuner(std::shared_ptr<PitchShift> ps) : IAudioFX(), ps(ps)
{
    paramMap["Notes"] = {"FXKeyPicker", "", "Set which notes to tune to", 0, 0};
}

void Autotuner::Process(float *buf) {
    if ((!ps->getPitchshift() && ps->getAutotune()) || (ps->getPitchshift() && ps->getAutotune()))
        ps->repitch(buf);
}

void Autotuner::Reset() {
    ps->reset();
}

float Autotuner::Get(std::string item) {
    if (item == "Notes") {
        std::vector<bool> notes = ps->getNotes();
        int bitmask = 0, inc = 1;
        for (bool note : notes) {
            if (note) bitmask += inc;
            inc *= 2;
        }
        return bitmask; // will need to be rounded when received on the other end
    } else if (item == "Speed") {
        //return ps->getSpeed();
    }
    return 0.f;
}

void Autotuner::Set(std::string item, int val) {
    if (item == "Notes") {
        std::vector<bool> notes(12, false);
        int inc = pow(2, 11), bitmask = val;
        for (int i = 11; i >= 0; i--) {
            if (bitmask >= pow(2, i)) {
                bitmask -= inc;
                notes[i] = true;
            }
            inc /= 2;
        }
        ps->setNotes(notes);
    } else if (item == "Speed") {}
        //ps->setSpeed(val / 100.f);
}

bool Autotuner::GetEnabled() {
    return ps->getAutotune();
}

void Autotuner::SetEnabled(bool enabled) {
    ps->setAutotune(enabled);
}
