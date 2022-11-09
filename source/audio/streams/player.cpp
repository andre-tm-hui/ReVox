#include "player.h"

/* The player object, dedicated to loading a file and playing it to a specified output device. */

Player::Player(device outputDevice, int sampleRate, int framesPerBuffer, std::string dir, float *playbackBuffer)
    : AudioStream({-1, -1}, outputDevice, sampleRate, framesPerBuffer, dir)
{
    initialSetup = false;

    data.buf = playbackBuffer;

    files = {};
    data.timers = {};
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

void Player::Play(int keycode, json settings)
{
    // check if a file corresponding to the keycode exists
    std::string FILE_NAME = dir + "/samples/" + std::to_string(keycode) + ".mp3";
    if (!std::filesystem::exists(FILE_NAME.c_str()))
    {
        return;
    }

    // add a new list if the hotkey hasn't been played yet
    if (files.find(keycode) == files.end())
    {
        files[keycode] = {};
    }

    // check if the hotkey limit is reached
    if (files[keycode].size() >= settings["maxCopies"])
    {
        // Cleanup first item of data.files[keycode]
        SNDFILE* clear = files[keycode].front();
        files[keycode].pop();
        if (data.timers.contains(clear))
        {
            data.timers[clear].timeLeft = -1;
        }
    }

    // open the file and add it to the queue
    SNDFILE* file = sf_open(FILE_NAME.c_str(), SFM_READ, &data.info);
    if (settings["endAt"] == -1) settings["endAt"] = data.info.frames;
    float *temp = new float[2 * settings["startAt"].get<int>()];
    sf_read_float(file, temp, 2 * settings["startAt"].get<int>());
    delete[] temp;
    files[keycode].push(file);
    data.timers[file].timeLeft = settings["endAt"].get<int>() - settings["startAt"].get<int>();
    data.timers[file].volume = settings["volume"];
}

/* Utility function to check if a file corresponding to the keycode exists */
bool Player::CanPlay(int keycode)
{
    std::string FILE_NAME = dir + "samples/" + std::to_string(keycode) + ".mp3";
    return std::filesystem::exists(FILE_NAME.c_str());
}

void Player::StopAll()
{
    for (auto const& [file, time] : data.timers)
    {
        sf_close(file);
    }
    data.timers = {};
    files = {};
}

/* Utility function to rename a file if the keybind is being rebinded */
void Player::Rename(int keycodeFrom, int keycodeTo)
{
    std::filesystem::rename(dir + "samples/" + std::to_string(keycodeFrom) + ".mp3",
                            dir + "samples/" + std::to_string(keycodeTo) + ".mp3");
}
