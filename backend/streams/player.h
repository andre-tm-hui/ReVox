#ifndef PLAYER_H
#define PLAYER_H

#include <queue>
#include "audiostream.h"
#include "../nlohmann/json.hpp"

using namespace nlohmann;

class Player : public AudioStream
{
public:
    Player(device inputDevice,
           device outputDevice,
           int sampleRate,
           int framesPerBuffer,
           std::string dir,
           std::queue<float> *bufQueue);

    void Play(int keycode, json settings);

    bool CanPlay(int keycode);

    void StopAll();

    void Rename(int keycodeFrom, int keycodeTo);

    void Record(int keycode);
    void Stop();
    void Merge();
    void SetPadding(int padding);

private:
    std::map<int, std::queue<SNDFILE*>> files;

    RingBuffer<float> pBuf;
    int keycode,
        padding = 0;
    bool recording = false;

    int maxLiveSamples;

    playData data = {};
};

#endif // PLAYER_H
