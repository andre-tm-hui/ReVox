#ifndef PLAYER_H
#define PLAYER_H

#include <queue>

#include "json.hpp"
#include "audiostream.h"

using namespace nlohmann;

class Player : public AudioStream {
 public:
  Player(int inputDevice, int nInputChannels, int outputDevice, int nOutputChannels, int sampleRate,
         int framesPerBuffer, std::string dir, std::queue<float> *bufQueue);

  void Play(int idx, json settings);

  bool CanPlay(int idx);

  void StopAll();
  void StopPlaying(int idx);

  void Rename(int idxFrom, int idxTo);

  void Record(int idx);
  void Stop();
  void Merge();
  void SetPadding(int padding);

 private:
  std::map<int, std::queue<SNDFILE *>> files;

  RingBuffer<float> pBuf;
  int idx, padding = 0;
  bool recording = false;

  int maxLiveSamples;

  playData data = {};
};

#endif  // PLAYER_H
