#include "recorder.h"

/* The recorder object. In charge of recording audio from a given input device */

Recorder::Recorder(device inputDevice, int sampleRate, int framesPerBuffer, std::string dir, std::string appendToFname)
    : AudioStream(inputDevice, {-1, -1}, sampleRate, framesPerBuffer, dir)
{
    // append a suffix to the filename - used to makes sure the loopback recorder does not overwrite the input device recorder
    this->appendToFname = appendToFname;
}

void Recorder::Record(int keycode, bool padAudio, bool deleteFiles)
{
    data.pad = padAudio;
    // check if we're already recording, to avoid recording multiple files at once
    if (!recording) {
        // create a filename based on the keycode
        std::string FILE_NAME = dir + "/samples/" + std::to_string(keycode) + appendToFname + ".wav";

        if (deleteFiles)
        {
            std::filesystem::remove(dir + "/samples/" + std::to_string(keycode) + appendToFname + ".wav");
            std::filesystem::remove(dir + "/samples/" + std::to_string(keycode) + ".wav");
        }

        // set the sndfile info to be a .wav file
        this->data.info.samplerate = this->sampleRate;
        this->data.info.channels = this->inputParameters.channelCount;
        this->data.info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_32;

        // open the file
        this->data.file = sf_open(FILE_NAME.c_str(), SFM_WRITE, &this->data.info);
        if (sf_error(this->data.file) != SF_ERR_NO_ERROR) {
            fprintf(stderr, (sf_strerror(data.file)));
            return;
        }

        this->data.timeStamp = (float)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

        // open and start the stream to record from
        this->err = Pa_OpenStream(
                    &this->stream,
                    &this->inputParameters,
                    NULL,
                    this->sampleRate,
                    this->framesPerBuffer,
                    paClipOff,
                    recordCallback,
                    &this->data
                    );
        if (this->err != paNoError) {
            done(); return;
        }
        streamSetup = true;

        this->err = Pa_StartStream(stream);
        if (err != paNoError) {
            done(); return;
        }

        recording = true;
    }
}

void Recorder::Merge(int keycode)
{
    // only occurs on the loopback recorder object, where there is a file suffix
    if (appendToFname != "" && std::filesystem::exists(dir + "/samples/" + std::to_string(keycode) + ".wav"))
    {
        // load both recorded files, add them together and output to a new file
        SndfileHandle fileA, fileB, fileOut;
        short bufferA[1024], bufferB[1024], bufferOut[1024];

        fileA = SndfileHandle(dir + "/samples/" + std::to_string(keycode) + appendToFname + ".wav");
        fileB = SndfileHandle(dir + "/samples/" + std::to_string(keycode) + ".wav");

        fileOut = SndfileHandle(
                    dir + "/samples/" + std::to_string(keycode) + "out.wav",
                    SFM_WRITE,
                    SF_FORMAT_WAV | SF_FORMAT_PCM_32,
                    2,
                    sampleRate);
        sf_count_t countA;
        sf_count_t countB;


        while (1) {
            countA = fileA.read(bufferA, 1024);
            countB = fileB.read(bufferB, 1024);

            // combine the two buffers and write to the output file
            for (int i = 0; i < (countA > countB ? countA : countB); i++) {
                short a = i < countA ? bufferA[i] : 0;
                short b = i < countB ? bufferB[i] : 0;
                bufferOut[i] = 0.5 * (a + b);
            }
            fileOut.write(bufferOut, 1024);

            // break the while loop if both files reach EOF
            if (countA < 1024 && countB < 1024) {
                break;
            }
        }
    }
    else
    {
        std::filesystem::rename((dir + "/samples/" + std::to_string(keycode) + appendToFname + ".wav").c_str(),
                                (dir + "/samples/" + std::to_string(keycode) + ".wav").c_str());
    }
    // cleanup
    if (std::filesystem::exists(dir + "/samples/" + std::to_string(keycode) + "out.wav"))
    {
        std::filesystem::remove((dir + "/samples/" + std::to_string(keycode) + ".wav").c_str());
        std::filesystem::remove((dir + "/samples/" + std::to_string(keycode) + appendToFname + ".wav").c_str());
        std::filesystem::rename((dir + "/samples/" + std::to_string(keycode) + "out.wav").c_str(),
                                (dir + "/samples/" + std::to_string(keycode) + ".wav").c_str());
    }
}

void Recorder::Stop(int keycode)
{
    // only call if currently recording
    if (recording) {
        // close the file being written to
        sf_close(data.file);

        // Close the stream
        err = Pa_CloseStream(stream);
        if (err != paNoError) {
            done(); return;
        }
        streamSetup = false;

        recording = false;

        if (appendToFname == "")
        {
            // if the input device is single-channel, we convert the output audio to dual-channel to prevent problems with playback
            if (inputParameters.channelCount == 1)
            {
                SndfileHandle fileIn, fileOut;
                short bufferIn[512], bufferOut[1024];

                fileIn = SndfileHandle(dir + "/samples/" + std::to_string(keycode) + appendToFname + ".wav");

                fileOut = SndfileHandle(
                            dir + "/samples/" + std::to_string(keycode) + "-dual.wav",
                            SFM_WRITE,
                            SF_FORMAT_WAV | SF_FORMAT_PCM_32,
                            2,
                            sampleRate);
                sf_count_t countIn;


                while (1) {
                    countIn = fileIn.read(bufferIn, 512);

                    // duplicate each element in the input buffer in an adjacent position
                    for (int i = 0; i < countIn; i++) {
                        bufferOut[(2 * i)] = bufferIn[i];
                        bufferOut[(2 * i) + 1] = bufferIn[i];
                    }
                    fileOut.write(bufferOut, 1024);

                    // break the while loop if both files reach EOF
                    if (countIn < 512) {
                        break;
                    }
                }
            }
            // some input devices, namely audio interfaces, record each connected device (e.g. mic and instrument) to separate channels, so we combine the channels and output to a dual-channel audio file
            else if (inputParameters.channelCount >= 2)
            {
                SndfileHandle fileIn, fileOut;
                short bufferIn[512 * inputParameters.channelCount], bufferOut[512 * 2];

                fileIn = SndfileHandle(dir + "/samples/" + std::to_string(keycode) + appendToFname + ".wav");

                fileOut = SndfileHandle(
                            dir + "/samples/" + std::to_string(keycode) + "-dual.wav",
                            SFM_WRITE,
                            SF_FORMAT_WAV | SF_FORMAT_PCM_32,
                            2,
                            sampleRate);
                sf_count_t countIn;


                while (1) {
                    countIn = fileIn.read(bufferIn, 1024);

                    // iterate through the read buffer, and combine every consecutive inputParameters.channelCount items
                    for (int i = 0; i < countIn / inputParameters.channelCount; i++) {
                        short toMono = 0;
                        for (int j = 0; j < inputParameters.channelCount; j++)
                        {
                            toMono += bufferIn[(inputParameters.channelCount * i) + j];
                        }
                        // write to a 2 channel buffer
                        bufferOut[2 * i] = toMono / inputParameters.channelCount;
                        bufferOut[(2 * i) + 1] = toMono / inputParameters.channelCount;
                    }
                    fileOut.write(bufferOut, 1024);

                    // break the while loop if both files reach EOF
                    if (countIn < 512 * inputParameters.channelCount) {
                        break;
                    }
                }
            }
            // cleanup
            if (std::filesystem::exists(dir + "/samples/" + std::to_string(keycode) + "-dual.wav"))
            {
                std::filesystem::remove((dir + "/samples/" + std::to_string(keycode) + ".wav").c_str());
                std::filesystem::rename((dir + "/samples/" + std::to_string(keycode) + "-dual.wav").c_str(),
                                        (dir + "/samples/" + std::to_string(keycode) + ".wav").c_str());
            }
        }
    }
}

