#include "audiostream.h"

/* The default/base portaudio stream object. All other portaudio stream objects
 * (recorder, player, passthrough) inherit this class */

AudioStream::AudioStream(int inputDevice, int nInputChannels, int outputDevice, int nOutputChannels,
                         int sampleRate, int framesPerBuffer, std::string dir,
                         std::string objType) : 
    LoggableObject(objType), 
    inputDevice(inputDevice), 
    nInputChannels(nInputChannels),
    outputDevice(outputDevice),
    nOutputChannels(nOutputChannels),
    dir(dir), 
    sampleRate(sampleRate), 
    framesPerBuffer(framesPerBuffer)
{
  // setup the stream parameters of each device
  if (inputDevice >= 0) {
    setupDevice(&inputParameters, inputDevice, nInputChannels, true);
  }
  if (outputDevice >= 0) {
    setupDevice(&outputParameters, outputDevice, nOutputChannels, false);
  }

  initialSetup = true;
  log(INFO, "AudioStream setup");
}

AudioStream::~AudioStream() {
  while (!initialSetup) {
  };
  done();
}

void AudioStream::setupDevice(PaStreamParameters* parameters, int device, int nChannels, bool isInput) {
  log(INFO, "AudioStream setupDevice() called");
  parameters->device = device;
  parameters->channelCount = nChannels > 2 ? 2 : nChannels;
  parameters->sampleFormat = PA_SAMPLE_TYPE;
  parameters->suggestedLatency =
      isInput ? Pa_GetDeviceInfo(device)->defaultLowInputLatency
              : Pa_GetDeviceInfo(device)->defaultLowOutputLatency;
  parameters->hostApiSpecificStreamInfo = NULL;
  if (isInput) {
    input = true;
  } else {
    output = true;
  }
}

void AudioStream::done() {
  // an error has occured somewhere, so we close the stream if available
  log(INFO, "Done called, terminating");
  if (err != paNoError) {
    log(ERR,
        "PortAudio error " + std::to_string(err) + ": " + Pa_GetErrorText(err));
    err = 1; /* Always return 0 or 1, but no other return codes. */
  }

  if (streamSetup) {
    Pa_CloseStream(stream);
  }

  initialSetup = false;
}
