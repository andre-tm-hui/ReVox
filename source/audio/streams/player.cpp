#include "player.h"

/* The player object, dedicated to loading a file and playing it to a specified output device. */

Player::Player(device outputDevice, int sampleRate, int framesPerBuffer, std::string dir, int maxLiveSamples)
    : AudioStream({-1, -1}, outputDevice, sampleRate, framesPerBuffer, dir)
{
    initialSetup = false;
    this->maxLiveSamples = maxLiveSamples;
    data.maxFileLength = 10;
    data.files = new std::map<SNDFILE*, int>();

    // written files are set to always have 2 channels
    SF_INFO info;
    info.channels = 2;
    info.samplerate = this->sampleRate;
    data.info = info;

    // open a stream, as player is expected to always be ready to play something
    err = Pa_OpenStream(
                &stream,
                NULL,
                &outputParameters,
                this->sampleRate,
                this->framesPerBuffer,
                paClipOff,
                playCallback,
                &this->data
                );
    if (err != paNoError)
    {
        done(); return;
    }
    streamSetup = true;

    this->err = Pa_StartStream(this->stream);
    if (this->err != paNoError)
    {
        done();
    }
    initialSetup = true;
}

void Player::Play(int keycode)
{
    // check if a file corresponding to the keycode exists
    std::string FILE_NAME = dir + "/samples/" + std::to_string(keycode) + ".wav";
    if (!std::filesystem::exists(FILE_NAME.c_str()))
    {
        return;
    }
    // open the file and add it to the queue
    /* TODO:
     * dynamic changing of maxLiveSamples and maxFileLength
     * toggle between playing clips over each other and stopping the previous clip to play the next clip (maxLiveSamples = 1?)
     */
    SNDFILE* file = sf_open(FILE_NAME.c_str(), SFM_READ, &data.info);
    if ((int)data.files->size() < maxLiveSamples) {
        (*data.files)[file] = 0;
    }
}

/* Utility function to check if a file corresponding to the keycode exists */
bool Player::CanPlay(int keycode)
{
    std::string FILE_NAME = dir + "samples/" + std::to_string(keycode) + ".wav";
    return std::filesystem::exists(FILE_NAME.c_str());
}

/* Utility function to rename a file if the keybind is being rebinded */
void Player::Rename(int keycodeFrom, int keycodeTo)
{
    std::filesystem::rename(dir + "samples/" + std::to_string(keycodeFrom) + ".wav",
                            dir + "samples/" + std::to_string(keycodeTo) + ".wav");
}
