#include "audiostream.h"

/* The default/base portaudio stream object. All other portaudio stream objects
 * (recorder, player, passthrough) inherit this class */

AudioStream::AudioStream(device inputDevice, device outputDevice,
                         int sampleRate, int framesPerBuffer, std::string dir,
                         std::string objType)
    : LoggableObject(objType) {
  // set some properties of the object
  this->sampleRate = sampleRate;
  this->framesPerBuffer = framesPerBuffer;
  this->dir = dir;
  this->inputDevice = inputDevice.id;
  this->outputDevice = outputDevice.id;

  // setup the stream parameters of each device
  if (inputDevice.id >= 0) {
    setupDevice(&inputParameters, inputDevice, true);
  }
  if (outputDevice.id >= 0) {
    setupDevice(&outputParameters, outputDevice, false);
  }

  initialSetup = true;
  log(INFO, "AudioStream setup");
}

AudioStream::~AudioStream() {
  while (!initialSetup) {
  };
  done();
}

void AudioStream::setupDevice(PaStreamParameters* parameters, device device,
                              bool isInput) {
  log(INFO, "AudioStream setupDevice() called");
  parameters->device = device.id;
  parameters->channelCount = device.nChannels > 2 ? 2 : device.nChannels;
  parameters->sampleFormat = PA_SAMPLE_TYPE;
  parameters->suggestedLatency =
      isInput ? Pa_GetDeviceInfo(device.id)->defaultLowInputLatency
              : Pa_GetDeviceInfo(device.id)->defaultLowOutputLatency;
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
