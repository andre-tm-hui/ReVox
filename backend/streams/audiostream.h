#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#include <windows.h>

#include <cstdint>
#include <filesystem>
#include <fstream>

#include "../../util/loggableobject.h"
#include "callbacks.h"

#define PA_SAMPLE_TYPE paFloat32
typedef float SAMPLE;

typedef struct {
  std::map<std::string, int> ids;
  //int id;
  int nChannels;
} device;

class AudioStream : public LoggableObject {
 public:
  AudioStream(int inputDevice, int nInputChannels, int outputDevice, int nOutputChannels, int sampleRate,
              int framesPerBuffer, std::string dir, std::string objType);
  ~AudioStream();

  int inputDevice, nInputChannels, outputDevice, nOutputChannels, sampleRate, sampleSize, framesPerBuffer;
  void done();

  PaStreamParameters inputParameters, outputParameters;
  PaStream* stream;
  bool streamSetup = false;
  PaError err = paNoError;

  void setupDevice(PaStreamParameters* parameters, int device, int nChannels, bool isInput = true);

  std::string dir;

  bool initialSetup = false, input = false, output = false;
};

#endif  // AUDIOSTREAM_H
