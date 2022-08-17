#include "player.h"

/* The player object, dedicated to loading a file and playing it to a specified output device. */

Player::Player(device outputDevice, int sampleRate, int framesPerBuffer, std::string dir, float *playbackBuffer)
    : AudioStream({-1, -1}, outputDevice, sampleRate, framesPerBuffer, dir)
{
    initialSetup = false;

    data.buf = playbackBuffer;

    data.files = new std::map<SNDFILE*, int>();
    data.queue = new std::vector<SNDFILE*>();

    // written files are set to always have 2 channels
    SF_INFO info;
    info.channels = 2;
    info.samplerate = this->sampleRate;
    data.info = info;

    // open a stream, as player is expected to always be ready to play something
    err = Pa_OpenStream(
                &stream,
                NULL,
                output ? &this->outputParameters : NULL,
                this->sampleRate,
                this->framesPerBuffer,
                paClipOff,
                output ? playCallback : NULL,
                &this->data
                );
    if (err != paNoError)
    {
        std::cout<<Pa_GetErrorText(this->err)<<std::endl;
        done(); return;
    }
    streamSetup = true;

    this->err = Pa_StartStream(this->stream);
    if (this->err != paNoError)
    {
        std::cout<<Pa_GetErrorText(this->err)<<std::endl;
        done();
    }
    initialSetup = true;
}

void Player::Play(int keycode)
{
    // check if a file corresponding to the keycode exists
    std::string FILE_NAME = dir + "/samples/" + std::to_string(keycode) + ".mp3";
    if (!std::filesystem::exists(FILE_NAME.c_str()))
    {
        return;
    }
    // open the file and add it to the queue
    SNDFILE* file = sf_open(FILE_NAME.c_str(), SFM_READ, &data.info);
    if ((int)data.files->size() >= maxLiveSamples) {
        SNDFILE* erase = (*data.queue)[0];
        (*data.files).erase(erase);
        (*data.queue).erase((*data.queue).begin());
        if (erase != nullptr) sf_close(erase);
    }
    (*data.files)[file] = 0;
    (*data.queue).push_back(file);
}

/* Utility function to check if a file corresponding to the keycode exists */
bool Player::CanPlay(int keycode)
{
    std::string FILE_NAME = dir + "samples/" + std::to_string(keycode) + ".mp3";
    return std::filesystem::exists(FILE_NAME.c_str());
}

/* Utility function to rename a file if the keybind is being rebinded */
void Player::Rename(int keycodeFrom, int keycodeTo)
{
    std::filesystem::rename(dir + "samples/" + std::to_string(keycodeFrom) + ".mp3",
                            dir + "samples/" + std::to_string(keycodeTo) + ".mp3");
}
