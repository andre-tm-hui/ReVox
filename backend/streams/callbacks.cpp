#include "callbacks.h"

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
    for (auto it = p_data->timers.begin(); it != p_data->timers.end();)
    {
        SNDFILE* file = it->first;
        soundData sdata = it->second;

        if (sdata.timeLeft <= 0)
        {
            sf_close(file);
            it = p_data->timers.erase(it);
        }
        else
        {
            // read the frames to a temporary read buffer
            num_read = sf_read_float(file, read, framesPerBuffer * 2);
            // add the read buffer to the aggregate read buffer
            for (int i = 0; i < num_read; i++) {
                *(out + i) += *(read + i) * sdata.volume;
            }

            p_data->timers[file].timeLeft -= framesPerBuffer;

            // close the file if EOF is reached
            if (num_read < framesPerBuffer)
            {
                sf_close(file);
                it = p_data->timers.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    // unassign the memory allocation for the read buffer
    delete[] read;
    memcpy(p_data->buf, out, sizeof(float) * framesPerBuffer * 2);
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
    for (int i = 0; i < framesPerBuffer; i++) {
        mono[i] = 0.f;
        for (int j = 0; j < p_data->nChannels; ++j) mono[i] += in[p_data->nChannels*i+j];
    }

    if (p_data->fxs != nullptr)
        for (auto &[key,fx] : *(p_data->fxs))
            if (fx->GetEnabled())
                fx->Process(mono);

    for (int i = 0; i < (int)framesPerBuffer; i++) {
        out[2*i] = mono[i];
        out[2*i+1] = mono[i];
    }

    delete[] mono;
    if (p_data->rData->inUse) {
        if (p_data->rData->file != nullptr)
            sf_write_float(p_data->rData->file, out, framesPerBuffer * p_data->rData->info.channels);
    } else {
        p_data->pBuf->write(out, framesPerBuffer * p_data->rData->info.channels);
    }

    memcpy(p_data->buf, out, sizeof(float) * framesPerBuffer * 2);
    return paContinue;
}


int monitorCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* data)
{
    // set some variables
    monitorData *c_data = (monitorData*) data;
    float* in = (float*)inputBuffer;
    float* out = (float*)outputBuffer;

    // record if flag is set
    if (c_data->rData->inUse) {
        if (c_data->rData->file != nullptr)
            sf_write_float(c_data->rData->file, in, framesPerBuffer * c_data->rData->info.channels);
    } else {
        c_data->pBuf->write(in, framesPerBuffer * c_data->rData->info.channels);
    }

    // copy the input (loopback) to a shared buffer, going to another stream
    memset(out, 0, sizeof(float) * framesPerBuffer * 2);

    // add the monitoring streams, multiplied by their volume
    for (int i = 0; i < framesPerBuffer * 2; i++)
    {
        out[i] += c_data->playbackBuffer[i] * c_data->monitorSamples;
        out[i] += c_data->inputBuffer[i] * c_data->monitorMic;
    }

    return paContinue;
}

int streamCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* data)
{
    float *in = (float*) inputBuffer;
    float *out = (float*) outputBuffer;
    memcpy(out, in, sizeof(float) * framesPerBuffer * 2);
    return paContinue;
}

int noiseCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* data)
{
    float *out = (float*) outputBuffer;
    for (int i = 0; i < framesPerBuffer; i++)
    {
        out[2*i] = i % 2 == 0 ? 0.0000001f : -0.0000001f;
        out[2*i+1] = i % 2 == 0 ? 0.0000001f : -0.0000001f;
    }
    return paContinue;
}
