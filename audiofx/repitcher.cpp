#include "audiofx/repitcher.h"

Repitcher::Repitcher(std::shared_ptr<PitchShifter> ps) : IAudioFX(), ps(ps)
{
    paramMap["Pitch"] = {"FXDial", "st", "Change the pitch in semitones", -12, 12};
    paramMap["Resample"] = {"FXSwitch", "", "Resample to change how your voice sounds repitched. Turn it off if you want your voice to still be recognizable.", 0, 0};
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
    else if (item == "Resample")
        return ps->getRepitchResample() ? 1.f : 0.f;

    return 0.f;
}

void Repitcher::Set(std::string item, int val) {
    if (item == "Pitch")
        ps->setRepitchFactor(pow(2, (float)val / 12.f)); // convert val (semitones) to frequency scalar
    else if (item == "Resample")
        ps->setRepitchResample((bool)val);
}

bool Repitcher::GetEnabled() {
    return ps->getRepitchActive();
}

void Repitcher::SetEnabled(bool enabled) {
    ps->setRepitchActive(enabled);
}
