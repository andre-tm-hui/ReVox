#ifndef REPITCHER_H
#define REPITCHER_H

#include "iaudiofx.h"
#include "lib/pitchshift/pitchshifter.h"

class Repitcher : public IAudioFX
{
public:
    Repitcher(std::shared_ptr<PitchShifter> ps);

    void Process(float *buf) override;
    void Reset(int framesPerBuffer = -1) override;
    float Get(std::string item) override;
    void Set(std::string item, int val) override;
    bool GetEnabled() override;
    void SetEnabled(bool enabled) override;

private:
    std::shared_ptr<PitchShifter> ps;
};

#endif // REPITCHER_H
