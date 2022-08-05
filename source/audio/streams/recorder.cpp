#include "recorder.h"

/* The recorder object. In charge of recording audio from a given input device */

Recorder::Recorder(device inputDevice, int sampleRate, int framesPerBuffer, std::string dir, std::string appendToFname)
    : AudioStream(inputDevice, {-1, -1}, sampleRate, framesPerBuffer, dir)
{
    initialSetup = false;
    // append a suffix to the filename - used to makes sure the loopback recorder does not overwrite the input device recorder
    this->appendToFname = appendToFname;

    this->data.inUse = false;
    initialSetup = true;
}

void Recorder::Record(int keycode, bool padAudio, bool deleteFiles)
{
    data.pad = padAudio;
    // check if we're already recording, to avoid recording multiple files at once
    if (!recording) {
        // create a filename based on the keycode
        std::string FILE_NAME = dir + "/samples/" + std::to_string(keycode) + appendToFname + ".mp3";

        if (deleteFiles)
        {
            std::filesystem::remove(dir + "/samples/" + std::to_string(keycode) + appendToFname + ".mp3");
            std::filesystem::remove(dir + "/samples/" + std::to_string(keycode) + ".mp3");
        }

        // set the sndfile info to be a .mp3 file
        this->data.info.samplerate = this->sampleRate;
        this->data.info.channels = this->inputParameters.channelCount;
        this->data.info.format = SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_III;

        // open the file
        this->data.file = sf_open(FILE_NAME.c_str(), SFM_WRITE, &this->data.info);
        if (sf_error(this->data.file) != SF_ERR_NO_ERROR) {
            fprintf(stderr, (sf_strerror(data.file)));
            return;
        }
        this->data.inUse = true;

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
            std::cout<<Pa_GetErrorText(this->err)<<std::endl;
            done(); return;
        }
        streamSetup = true;

        this->err = Pa_StartStream(stream);
        if (err != paNoError) {
            std::cout<<Pa_GetErrorText(this->err)<<std::endl;
            done(); return;
        }

        recording = true;
    }
}

void Recorder::Merge(int keycode)
{
    // only occurs on the loopback recorder object, where there is a file suffix
    if (appendToFname != "" && std::filesystem::exists(dir + "/samples/" + std::to_string(keycode) + ".mp3"))
    {
        // load both recorded files, add them together and output to a new file
        SNDFILE *fileA, *fileB, *fileOut;
        float bufferA[1024], bufferB[1024], bufferOut[1024];
        SF_INFO infoA = data.info, infoB = data.info, outfo = data.info;
        infoA.format = 0;
        outfo.channels = 2;

        fileA = sf_open((dir + "samples/" + std::to_string(keycode) + appendToFname + ".mp3").c_str(),
                         SFM_READ,
                         &infoA);

        fileB = sf_open((dir + "samples/" + std::to_string(keycode) + ".mp3").c_str(),
                         SFM_READ,
                         &infoB);

        fileOut = sf_open((dir + "samples/" + std::to_string(keycode) + "out.mp3").c_str(),
                          SFM_WRITE,
                          &data.info);

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
    }
    else
    {
        std::filesystem::rename((dir + "/samples/" + std::to_string(keycode) + appendToFname + ".mp3").c_str(),
                                (dir + "/samples/" + std::to_string(keycode) + ".mp3").c_str());
    }
    // cleanup
    if (std::filesystem::exists(dir + "/samples/" + std::to_string(keycode) + "out.mp3"))
    {
        std::filesystem::remove((dir + "/samples/" + std::to_string(keycode) + ".mp3").c_str());
        std::filesystem::remove((dir + "/samples/" + std::to_string(keycode) + appendToFname + ".mp3").c_str());
        std::filesystem::rename((dir + "/samples/" + std::to_string(keycode) + "out.mp3").c_str(),
                                (dir + "/samples/" + std::to_string(keycode) + ".mp3").c_str());
    }
}

void Recorder::Stop(int keycode)
{
    while (data.inUse) {}
    // only call if currently recording
    if (recording) {
        // close the file being written to
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        streamSetup = false;
        sf_close(data.file);
        fprintf(stdout, "stop\n"); fflush(stdout);

        recording = false;

        if (appendToFname == "")
        {
            // if the input device is single-channel, we convert the output audio to dual-channel to prevent problems with playback
            if (inputParameters.channelCount == 1)
            {
                SNDFILE *fileIn, *fileOut;
                float bufferIn[512], bufferOut[512 * 2];
                SF_INFO info = data.info, outfo = data.info;
                info.format = 0;
                outfo.channels = 2;

                fileIn = sf_open((dir + "samples/" + std::to_string(keycode) + appendToFname + ".mp3").c_str(),
                                 SFM_READ,
                                 &info);

                fileOut = sf_open((dir + "samples/" + std::to_string(keycode) + appendToFname + "-dual.mp3").c_str(),
                                  SFM_WRITE,
                                  &data.info);

                sf_count_t countIn;


                while (1) {
                    countIn = sf_read_float(fileIn, bufferIn, 512);

                    if (countIn == 0) break;

                    // duplicate each element in the input buffer in an adjacent position
                    for (int i = 0; i < countIn; i++) {
                        bufferOut[(2 * i)] = bufferIn[i];
                        bufferOut[(2 * i) + 1] = bufferIn[i];
                    }
                    sf_write_float(fileOut, bufferOut, countIn * 2);

                    // break the while loop if both files reach EOF
                    if (countIn < 512) {
                        break;
                    }
                }

                sf_close(fileIn);
                sf_close(fileOut);
            }
            // some input devices, namely audio interfaces, record each connected device (e.g. mic and instrument) to separate channels, so we combine the channels and output to a dual-channel audio file
            else if (inputParameters.channelCount >= 2)
            {
                SNDFILE *fileIn, *fileOut;
                float bufferIn[512 * inputParameters.channelCount], bufferOut[512 * 2];
                SF_INFO info = data.info, outfo = data.info;
                info.format = 0;
                outfo.channels = 2;

                fileIn = sf_open((dir + "samples/" + std::to_string(keycode) + appendToFname + ".mp3").c_str(),
                                 SFM_READ,
                                 &info);

                fileOut = sf_open((dir + "samples/" + std::to_string(keycode) + appendToFname + "-dual.mp3").c_str(),
                                  SFM_WRITE,
                                  &outfo);

                sf_count_t countIn;

                while (1)
                {
                    countIn = sf_read_float(fileIn, bufferIn, 512 * inputParameters.channelCount);

                    if (countIn == 0) break;

                    // iterate through the read buffer, and combine every consecutive inputParameters.channelCount items
                    for (int i = 0; i < countIn / inputParameters.channelCount; i++) {
                        float toMono = 0;
                        for (int j = 0; j < inputParameters.channelCount; j++)
                        {
                            toMono += bufferIn[(inputParameters.channelCount * i) + j];
                        }
                        // write to a 2 channel buffer
                        bufferOut[2 * i] = toMono / inputParameters.channelCount;
                        bufferOut[(2 * i) + 1] = toMono / inputParameters.channelCount;
                    }

                    //std::cout<<2 * countIn / inputParameters.channelCount<<std::endl;
                    sf_write_float(fileOut, bufferOut, 2 * countIn / inputParameters.channelCount);

                    // break the while loop if both files reach EOF
                    if (countIn < 512 * inputParameters.channelCount) {
                        break;
                    }
                }

                sf_close(fileIn);
                sf_close(fileOut);
            }
            // cleanup
            if (std::filesystem::exists(dir + "/samples/" + std::to_string(keycode) + "-dual.mp3"))
            {
                std::filesystem::remove((dir + "/samples/" + std::to_string(keycode) + ".mp3").c_str());
                std::filesystem::rename((dir + "/samples/" + std::to_string(keycode) + "-dual.mp3").c_str(),
                                        (dir + "/samples/" + std::to_string(keycode) + ".mp3").c_str());
            }
        }
    }
}

