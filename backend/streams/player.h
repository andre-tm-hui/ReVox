#ifndef PLAYER_H
#define PLAYER_H

#include <queue>
#include "audiostream.h"
#include "../nlohmann/json.hpp"

using namespace nlohmann;

class Player : public AudioStream
{
public:
    Player(device outputDevice,
           int sampleRate,
           int framesPerBuffer,
           std::string dir,
           float *playbackBuffer);

    //~Player() { done(); }

    void Play(int keycode, json settings);

    bool CanPlay(int keycode);

    void StopAll();

    void Rename(int keycodeFrom, int keycodeTo);

    int maxLiveSamples;

    playData data = {};

private:
    std::map<int, std::queue<SNDFILE*>> files;
};

#endif // PLAYER_H
