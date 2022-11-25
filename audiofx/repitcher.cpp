#include "audiofx/repitcher.h"

Repitcher::Repitcher(std::shared_ptr<PitchShift> ps) : IAudioFX(), ps(ps)
{
    paramMap["Pitch"] = {"FXDial", "st", "Change the pitch in semitones", -12, 12};
}

void Repitcher::Process(float *buf) {
    if (!ps->getAutotune() && ps->getPitchshift())
        ps->repitch(buf);
}

void Repitcher::Reset() {
    ps->reset();
}

float Repitcher::Get(std::string item) {
    if (item == "Pitch")
        return ps->getPitchscale(); // will need to be rounded when received on the other end

    return 0.f;
}

void Repitcher::Set(std::string item, int val) {
    if (item == "Pitch")
        ps->setPitchscale(pow(2, val / 12)); // convert val (semitones) to frequency scalar
}

bool Repitcher::GetEnabled() {
    return ps->getPitchshift();
}

void Repitcher::SetEnabled(bool enabled) {
    ps->setPitchshift(enabled);
}
