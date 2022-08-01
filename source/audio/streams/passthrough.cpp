#include "passthrough.h"

/* The passthrough stream object, writes audio from the input device directly to the output device. */

Passthrough::Passthrough(device inputDevice, device outputDevice, int sampleRate, int framesPerBuffer)
    : AudioStream(inputDevice, outputDevice, sampleRate, framesPerBuffer, "")
{
    initialSetup = false;
    // start a stream on creation of this object, since audio from the default input should always be written to the virtual cable input
    this->err = Pa_OpenStream(
                &this->stream,
                &inputParameters,
                &outputParameters,
                this->sampleRate,
                this->framesPerBuffer,              // this has to be as low as possible, and non-zero, because copying a large buffer introduces audio distortion
                paClipOff,
                passthroughCallback,
                &this->data
                );
    streamSetup = true;
    if (this->err != paNoError)
    {
        done(); return;
    }

    this->err = Pa_StartStream(this->stream);
    if (this->err != paNoError)
    {
        done(); return;
    }

    data.reverb = new revmodel();
    data.useReverb = false;

    data.ps = new PitchShift(framesPerBuffer, sampleRate);
    data.useAutotune = false;

    initialSetup = true;
}

void Passthrough::SetFX(json settings)
{
    data.useReverb = settings["reverb"]["enabled"].get<bool>();
    (*checkboxes)["reverb"]->setCheckState(data.useReverb ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    data.reverb->setwet(settings["reverb"]["mix"].get<float>());
    data.reverb->setdry(1.f - settings["reverb"]["mix"].get<float>());
    data.reverb->setdamp(settings["reverb"]["damp"].get<float>());
    data.reverb->setroomsize(settings["reverb"]["roomsize"].get<float>());
    data.reverb->setwidth(settings["reverb"]["width"].get<float>());

    data.useAutotune = settings["autotune"]["enabled"].get<bool>();
    (*checkboxes)["autotune"]->setCheckState(data.useAutotune ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    //data.ps.setSpeed(settings["autotune"]["speed"].get<float>();
}
