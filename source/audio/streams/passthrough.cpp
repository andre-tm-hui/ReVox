#include "passthrough.h"

/* The passthrough stream object, writes audio from the input device directly to the output device. */

Passthrough::Passthrough(device inputDevice, device outputDevice, int sampleRate, int framesPerBuffer)
    : AudioStream(inputDevice, outputDevice, sampleRate, framesPerBuffer, "")
{
    // start a stream on creation of this object, since audio from the default input should always be written to the virtual cable input
    this->err = Pa_OpenStream(
                &this->stream,
                &inputParameters,
                &outputParameters,
                this->sampleRate,
                1,              // this has to be as low as possible, and non-zero, because copying a large buffer introduces audio distortion
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
    fprintf(stdout, "reverberator made\n"); fflush(stdout);
    data.useReverb = false;
    //data.reverb->SetDelay(300);
}
