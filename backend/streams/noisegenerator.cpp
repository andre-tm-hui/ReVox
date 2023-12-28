#include "noisegenerator.h"

NoiseGenerator::NoiseGenerator(int outputDevice, int nChannels, int sampleRate)
    : AudioStream(-1, -1, outputDevice, nChannels, sampleRate, 2, "", "NoiseGenerator") {
  initialSetup = false;
  // open and start the stream to record from
  this->err = Pa_OpenStream(
      &this->stream, NULL, output ? &this->outputParameters : NULL,
      this->sampleRate, 2, paClipOff, output ? noiseCallback : NULL, nullptr);

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

  log(INFO, "NoiseGenerator stream setup");
}
