#ifndef REVERBERATOR_H
#define REVERBERATOR_H

#include "iaudiofx.h"
#include "lib/reverb/revmodel.h"

class Reverberator : public IAudioFX
{
public:
    Reverberator(int bufSize);

    void Process(float *buf) override;
    void Reset() override;
    float Get(std::string item) override;
    void Set(std::string item, int val) override;
    bool GetEnabled() override;
    void SetEnabled(bool enabled) override;

private:
    std::unique_ptr<revmodel> rev;
    int bufSize;
};

#endif // REVERBERATOR_H
