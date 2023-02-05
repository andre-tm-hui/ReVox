#include "audiofx/repitcher.h"

Repitcher::Repitcher(std::shared_ptr<PitchShifter> ps) : IAudioFX(), ps(ps)
{
    paramMap["Pitch"] = {"FXDial", "st", "Change the pitch in semitones", -12, 12};
}

void Repitcher::Process(float *buf) {
    if (!ps->getAutotuneActive() && ps->getRepitchActive())
        ps->process(buf);
}

void Repitcher::Reset(int framesPerBuffer) {
    ps->reset();
}

float Repitcher::Get(std::string item) {
    if (item == "Pitch")
        return ps->getRepitchFactor(); // will need to be rounded when received on the other end

    return 0.f;
}

void Repitcher::Set(std::string item, int val) {
    if (item == "Pitch")
        ps->setRepitchFactor(pow(2, val / 12)); // convert val (semitones) to frequency scalar
}

bool Repitcher::GetEnabled() {
    return ps->getRepitchActive();
}

void Repitcher::SetEnabled(bool enabled) {
    ps->setRepitchActive(enabled);
}
