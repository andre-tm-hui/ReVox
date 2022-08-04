#ifndef RECORDER_H
#define RECORDER_H

#include "audiostream.h"
#include <sndfile.hh>
#include <cstdio>
#include <stdio.h>

class Recorder : public AudioStream
{
public:
    Recorder(device inputDevice,
             int sampleRate,
             int framesPerBuffer,
             std::string dir,
             std::string appendToFname = "");

    //~Recorder() { done(); }

    void Record(int keycode, bool padAudio = false, bool deleteFiles = false);
    void Stop(int keycode);
    void Merge(int keycode);

    recordData data = {};

private:
    bool recording = false;
    std::string appendToFname;
};

#endif // RECORDER_H
