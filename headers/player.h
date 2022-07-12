#ifndef PLAYER_H
#define PLAYER_H

#include "audiostream.h"

class Player : public AudioStream
{
public:
    Player(device outputDevice,
           int sampleRate,
           int framesPerBuffer,
           std::string dir,
           int maxLiveSamples = 5);

    void Play(int keycode);

    bool CanPlay(int keycode);

    void Rename(int keycodeFrom, int keycodeTo);

    int maxLiveSamples;
};

#endif // PLAYER_H
