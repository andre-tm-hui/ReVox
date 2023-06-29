#include "cleanoutput.h"

CleanOutput::CleanOutput(device inputDevice, device outputDevice,
                         int sampleRate, int framesPerBuffer)
    : AudioStream(inputDevice, outputDevice, sampleRate, framesPerBuffer, "",
                  "CleanOutput") {
  initialSetup = false;
  // open and start the stream to record from
  this->err = Pa_OpenStream(
      &this->stream, input ? &this->inputParameters : NULL,
      output ? &this->outputParameters : NULL, this->sampleRate, 1, paClipOff,
      input && output ? streamCallback : NULL, nullptr);

  if (this->err != paNoError) {
    done();
    return;
  }
  streamSetup = true;

  this->err = Pa_StartStream(stream);
  if (err != paNoError) {
    done();
    return;
  }
  initialSetup = true;
  log(INFO, "CleanOutput stream setup");
}
