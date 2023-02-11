#include "callbacks.h"

std::mutex m_input;
std::mutex m_playback;

int playCallback(const void* inputBuffer, void* outputBuffer,
                 unsigned long framesPerBuffer,
                 const PaStreamCallbackTimeInfo* timeInfo,
                 PaStreamCallbackFlags statusFlags,
                 void* data)
{
    m_playback.lock();
    // create and assign some variables
    playData* p_data = (playData*) data;
    float* in = (float*)inputBuffer;
    float* out = (float*)outputBuffer;
    float* read = new float[framesPerBuffer * 2];
    sf_count_t num_read;

    // record if flag is set
    if (p_data->rData->inUse) {
        if (p_data->rData->file != nullptr)
            sf_write_float(p_data->rData->file, in, framesPerBuffer * p_data->rData->info.channels);
    } else {
        p_data->pBuf->write(in, framesPerBuffer * p_data->rData->info.channels);
    }

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
                out[i] += read[i] * sdata.volume;
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

    for (int i = 0; i < framesPerBuffer * 2; i++) p_data->bufQueue->push(out[i]);
    if (p_data->bufQueue->size() > 3 * 2 * framesPerBuffer)
        for (int i = 0; i < framesPerBuffer * 2; i++)
            p_data->bufQueue->pop();
    // unassign the memory allocation for the read buffer
    delete[] read;
    m_playback.unlock();
    return paContinue;
}

int passthroughCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* data)
{
    m_input.lock();
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
        p_data->bufQueue->push(mono[i]);
        p_data->bufQueue->push(mono[i]);
        out[2*i] = mono[i];
        out[2*i+1] = mono[i];
    }

    delete[] mono;
    if (p_data->bufQueue->size() > 3 * 2 * framesPerBuffer)
        for (int i = 0; i < framesPerBuffer * 2; i++)
            p_data->bufQueue->pop();

    if (p_data->rData->inUse) {
        if (p_data->rData->file != nullptr)
            sf_write_float(p_data->rData->file, out, framesPerBuffer * p_data->rData->info.channels);
    } else {
        p_data->pBuf->write(out, framesPerBuffer * p_data->rData->info.channels);
    }
    m_input.unlock();
    return paContinue;
}


int monitorCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* data)
{
    m_input.lock();
    m_playback.lock();
    // set some variables
    monitorData *c_data = (monitorData*) data;
    float* in = (float*)inputBuffer;
    float* out = (float*)outputBuffer;

    memset(out, 0, sizeof(float) * framesPerBuffer * 2);
    // add the monitoring streams, multiplied by their volume
    for (int i = 0; i < framesPerBuffer * 2; i++)
    {
        if (!c_data->playbackQueue->empty()) {
            out[i] += c_data->playbackQueue->front() * c_data->monitorSamples;
            c_data->playbackQueue->pop();
        }
        if (!c_data->inputQueue->empty()) {
            out[i] += c_data->inputQueue->front() * c_data->monitorMic;
            c_data->inputQueue->pop();
        }
    }
    m_input.unlock();
    m_playback.unlock();

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
