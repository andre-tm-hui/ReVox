#include "monitor.h"

Monitor::Monitor(device inputDevice,
                 device outputDevice,
                 int sampleRate,
                 int framesPerBuffer,
                 std::string dir,
                 float *inputBuffer,
                 float *playbackBuffer) : AudioStream(inputDevice, outputDevice, sampleRate, framesPerBuffer, dir)
{
    initialSetup = false;
    this->data.inputBuffer = inputBuffer;
    this->data.playbackBuffer = playbackBuffer;
    this->data.rData = new recordData();
    this->data.rData->info = {};
    this->data.rData->inUse = false;
    this->data.monitorMic = 0.f;
    this->data.monitorSamples = 0.f;

    // open and start the stream to record from
    this->err = Pa_OpenStream(
                &this->stream,
                input ? &this->inputParameters : NULL,
                output ? &this->outputParameters : NULL,
                this->sampleRate,
                this->framesPerBuffer,
                paClipOff,
                input && output ? monitorCallback : NULL,
                &this->data
                );
    if (this->err != paNoError) {
        std::cout<<Pa_GetErrorText(this->err)<<std::endl;
        done(); return;
    }
    streamSetup = true;

    this->err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cout<<Pa_GetErrorText(this->err)<<std::endl;
        done(); return;
    }

    data.pBuf = &pBuf;
    pBuf.setSize(this->padding * (sampleRate / 1000));

    initialSetup = true;
}


void Monitor::Record(int keycode)
{
    if (!recording) {
        // create a filename based on the keycode
        std::string FILE_NAME = dir + "/samples/" + std::to_string(keycode) + "m.mp3";

        // set the sndfile info to be a .mp3 file
        this->data.rData->info.samplerate = this->sampleRate;
        this->data.rData->info.channels = 2;
        this->data.rData->info.format = SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_III;

        // open the file
        this->data.rData->file = sf_open(FILE_NAME.c_str(), SFM_WRITE, &this->data.rData->info);
        if (sf_error(this->data.rData->file) != SF_ERR_NO_ERROR) {
            fprintf(stderr, (sf_strerror(data.rData->file)));
            return;
        }
        this->data.rData->inUse = true;
        if (pBuf.len() > 0)
            sf_write_float(this->data.rData->file, pBuf.get(), pBuf.len());

        recording = true;
        this->keycode = keycode;
    }
}

void Monitor::Stop()
{
    // only call if currently recording
    if (recording) {
        // tell the callback to stop recording, and wait for thee callback to finish up any work
        this->data.rData->inUse = false;
        Sleep(200);
        // close the file being written to
        sf_close(data.rData->file);

        recording = false;

        Merge();
    }
}

void Monitor::Merge()
{
    if (!std::filesystem::exists((dir + "samples/" + std::to_string(keycode) + ".mp3").c_str()))
    {
        std::filesystem::rename((dir + "samples/" + std::to_string(keycode) + "m.mp3").c_str(),
                                (dir + "samples/" + std::to_string(keycode) + ".mp3").c_str());
        return;
    }
    // load both recorded files, add them together and output to a new file
    SNDFILE *fileA, *fileB, *fileOut;
    float bufferA[1024], bufferB[1024], bufferOut[1024];
    SF_INFO infoA = data.rData->info, infoB = data.rData->info;
    infoA.format = 0;

    fileA = sf_open((dir + "samples/" + std::to_string(keycode) + "m.mp3").c_str(),
                    SFM_READ,
                    &infoA);

    fileB = sf_open((dir + "samples/" + std::to_string(keycode) + ".mp3").c_str(),
                    SFM_READ,
                    &infoB);

    fileOut = sf_open((dir + "samples/" + std::to_string(keycode) + "out.mp3").c_str(),
                      SFM_WRITE,
                      &data.rData->info);

    sf_count_t countA, countB;

    while (1) {
        countA = sf_read_float(fileA, bufferA, 1024);
        countB = sf_read_float(fileB, bufferB, 1024);

        if (countA == 0 && countB == 0) break;

        // combine the two buffers and write to the output file
        for (int i = 0; i < (countA > countB ? countA : countB); i++) {
            float a = i < countA ? bufferA[i] : 0;
            float b = i < countB ? bufferB[i] : 0;
            bufferOut[i] = 0.5 * (a + b);
        }
        sf_write_float(fileOut, bufferOut, 1024);

        // break the while loop if both files reach EOF
        if (countA < 1024 && countB < 1024) {
            break;
        }
    }

    sf_close(fileA);
    sf_close(fileB);
    sf_close(fileOut);


    // cleanup
    if (std::filesystem::exists(dir + "/samples/" + std::to_string(keycode) + "out.mp3"))
    {
        std::filesystem::remove((dir + "/samples/" + std::to_string(keycode) + ".mp3").c_str());
        std::filesystem::remove((dir + "/samples/" + std::to_string(keycode) + "m.mp3").c_str());
        std::filesystem::rename((dir + "/samples/" + std::to_string(keycode) + "out.mp3").c_str(),
                                (dir + "/samples/" + std::to_string(keycode) + ".mp3").c_str());
    }
}

void Monitor::SetPadding(int padding)
{
    this->padding = padding;
    pBuf.setSize(this->padding * (sampleRate / 1000));
}
