#include <callbacks.h>

int recordCallback(const void* inputBuffer, void* outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* data)
{
    // create and assign some variables
    float* in = (float*)inputBuffer;
    recordData* p_data = (recordData*)data;
    //p_data->inUse = true;

    if (p_data->file != nullptr)
    {
        // check if we want to pad this audio source
        if (p_data->pad)
        {
            // create a buffer of 0s
            float* tempBuffer = new float[framesPerBuffer * p_data->info.channels];
            memset(tempBuffer, 0, sizeof(float) * framesPerBuffer * p_data->info.channels);

            // create some variables to track how much padding we've added
            float timePadded = 0;
            float currentTime = (float)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
            /* p_data holds a timestamp of the last time audio was actually written to file.
             * We find the amount of time that has elapsed since then, offset it by 1 buffer time-length,
             * and pad with 0s until the amount of time padded is equal or greater than the elapsed time. */
            while (timePadded < currentTime - p_data->timeStamp - (1000 * framesPerBuffer / p_data->info.samplerate))
            {
                timePadded += 1000 * framesPerBuffer / p_data->info.samplerate;
                sf_write_float(p_data->file, tempBuffer, framesPerBuffer * p_data->info.channels);
            }

            // clean up memory
            delete[] tempBuffer;
            // update the time stamp
            p_data->timeStamp = currentTime;
        }
        fprintf(stdout, "write\n"); fflush(stdout);
        // write the most recent buffer to file
        sf_write_float(p_data->file, in, framesPerBuffer * p_data->info.channels);
    }
    //p_data->inUse = false;
    return paContinue;
}

int playCallback(const void* inputBuffer, void* outputBuffer,
                 unsigned long framesPerBuffer,
                 const PaStreamCallbackTimeInfo* timeInfo,
                 PaStreamCallbackFlags statusFlags,
                 void* data)
{
    // create and assign some variables
    playData* p_data = (playData*) data;
    float* out = (float*)outputBuffer;
    float* read = new float[framesPerBuffer * 2];
    sf_count_t num_read;

    // clear the output buffer first
    memset(out, 0, sizeof(float) * framesPerBuffer * 2);
    // iterate through every file
    for (auto const& [file, timeAlive] : *p_data->files) {
        // files are available if timeAlive > -1
        if (timeAlive != -1) {
            // read the frames to a temporary read buffer
            num_read = sf_read_float(file, read, framesPerBuffer * 2);
            // add the read buffer to the aggregate read buffer
            for (int i = 0; i < num_read; i++) {
                *(out + i) += *(read + i);
            }

            // close the file if EOF is reached, or if the clip is too long, and remove the file from the map
            if (num_read < framesPerBuffer || timeAlive > p_data->maxFileLength * p_data->info.samplerate) {
                sf_close(file);
                (*p_data->files)[file] = -1;
                p_data->files->erase(file);
            }
            else {
                // keep track of how much of the file has been read
                (*p_data->files)[file] += framesPerBuffer;
            }
        }
    }

    // unassign the memory allocation for the read buffer
    delete[] read;
    return paContinue;
}

int passthroughCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* data)
{
    // declare and assign some variables
    float* in = (float*)inputBuffer;
    float* out = (float*)outputBuffer;
    passthroughData* p_data = (passthroughData*) data;

    float* mono = new float[framesPerBuffer];
    for (int i = 0; i < framesPerBuffer; i++)
    {
        mono[i] = (in[2*i] + in[2*i+1]);
    }

    if (p_data->pitchShift->getAutotune() || p_data->pitchShift->getPitchshift())
    {
        p_data->pitchShift->repitch(mono);
    }

    if (p_data->reverb->getEnabled())
    {
        p_data->reverb->processmono(mono, mono, framesPerBuffer, 1);
    }

    for (int i = 0; i < (int)framesPerBuffer; i++)
    {
        out[2*i] = mono[i];
        out[2*i+1] = mono[i];
    }

    delete[] mono;

    return paContinue;
}
