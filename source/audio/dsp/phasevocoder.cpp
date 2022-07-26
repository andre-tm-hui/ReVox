#include "phasevocoder.h"

PhaseVocoder::PhaseVocoder()
{
    zeroPadded = new float[this->bufSize * 2];
    autocorrFreqs = new fftwf_complex[this->bufSize + 1];
    autocorr = new float[this->bufSize * 2];

    in = new float[this->frameSize];
    freqs = new fftwf_complex[this->frameSize / 2 + 1];
    out = new float[this->frameSize];

    this->pf = fftwf_plan_dft_r2c_1d(this->frameSize, this->in, this->freqs, FFTW_ESTIMATE);
    this->pb = fftwf_plan_dft_c2r_1d(this->frameSize, this->freqs, this->out, FFTW_ESTIMATE);
    this->pfAuto = fftwf_plan_dft_r2c_1d(this->bufSize * 2, this->zeroPadded, this->autocorrFreqs, FFTW_ESTIMATE);
    this->pbAuto = fftwf_plan_dft_c2r_1d(this->bufSize * 2, this->autocorrFreqs, this->autocorr, FFTW_ESTIMATE);

    SetKey(this->key);
    this->freqPerBin = this->sampleRate / this->bufSize;
}

void PhaseVocoder::SetKey(Note key)
{
    this->key = key;
    auto scale = scales[this->key];
    this->notes = {};
    for (auto note : scale)
    {
        notes.push_back(noteFreqs[note]);
    }
    for (int i = 0; i < 5; i++)
    {
        for (auto note : scale)
        {
            if (i == 0)
            {
                this->notes.push_back(noteFreqs[note]);
            }
            else
            {
                this->notes.push_back(noteFreqs[note] / pow(2, i));
                this->notes.push_back(noteFreqs[note] * pow(2, i));
            }
        }
    }
    std::sort(notes.begin(), notes.end());
    fprintf(stdout, "key set\n"); fflush(stdout);
}

float PhaseVocoder::GetFreqScale(float* input)
{
    // Create a copy of the input buffer to window
    float* copy = new float[this->bufSize];
    memcpy(copy, input, sizeof(float) * this->bufSize);
    this->ApplyWindow(copy, this->bufSize);
    //fprintf(stdout, "windowed\n"); fflush(stdout);
    // Set the buffer to be FFT'd to all zeros
    memset(this->zeroPadded + this->bufSize, 0, sizeof(float) * this->bufSize);
    // Set the first half of the buffer to be the window, hence we have a zero-padded buffer, twice the buffer size i.e. twice the frequency resolution
    memcpy(this->zeroPadded, copy, sizeof(float) * this->bufSize);
    //fprintf(stdout, "0 padded\n"); fflush(stdout);
    delete[] copy;

    // Get the fundamental frequency
    // Apply FFT to original sample
    fftwf_execute(this->pfAuto);
    //fprintf(stdout, "fftd\n"); fflush(stdout);

    // Multiply by the complex conjugate to get the power spectrum
    /*for (int i = 0; i < this->bufSize+1; i++)
    {
        autocorrFreqs[i][0] = autocorrFreqs[i][0] * autocorrFreqs[i][0] + autocorrFreqs[i][1] * autocorrFreqs[i][1];
        autocorrFreqs[i][1] = 0.f;
    }
    //fprintf(stdout, "conjugate\n"); fflush(stdout);
    // Apply inverse FFT to get the autocorrelation
    fftwf_execute(this->pbAuto);
    fprintf(stdout, "autocorr\n"); fflush(stdout);

    // Find the highest amplitude, ignoring values outside the range of notes detected. This indicates the fundamental frequency
    float max = 0;
    int maxIx = 1;
    for (int i = 0; i < this->bufSize * 2; i++)
    {
        if (autocorr[i] > max && i > (float)this->sampleRate / notes[notes.size()-1] && i < (float)this->sampleRate / notes[0])
        {
            max = autocorr[i];
            maxIx = i;
        }
    }

    float fundamental = (float)this->sampleRate / (float)maxIx;*/

    float fundamental = 0;
    float maxM = 0;
    for (int i = 1; i < this->bufSize+1; i++)
    {
        float m = sqrt(autocorrFreqs[i][0] * autocorrFreqs[i][0] + autocorrFreqs[i][1] * autocorrFreqs[i][1]);
        float f = (float)i * ((float)this->sampleRate / ((float)this->bufSize * 2.f));
        if (m > maxM && f >= notes[0] && f <= notes[notes.size()-1])
        {
            maxM = m;
            fundamental = f;
        }
        //fprintf(stdout, "%f,", m); fflush(stdout);
    }

    // Find the target frequency by finding the closest frequency that matches a harmonic pitch
    float target = this->notes[0];
    float diff = abs(fundamental - this->notes[0]);
    for (auto &note : this->notes)
    {
        if (abs(fundamental - note) < diff)
        {
            target = note;
            diff = abs(fundamental - note);
        }
    }
    //fprintf(stdout, "freq scalar found\n"); fflush(stdout);
    fprintf(stdout, "\n%f %f\n", fundamental, target); fflush(stdout);
    return target / fundamental;
}

void PhaseVocoder::ApplyWindow(float* frame, int size)
{
    // apply a windowing function
    switch (this->windowType)
    {
    default:
        //fprintf(stdout, "windowing\n"); fflush(stdout);
        Window::Hann(frame, frame, size);
        break;
    }
}

std::vector<float> PhaseVocoder::ApplyWindow(std::vector<float> frame)
{
    // apply a windowing function
    switch (this->windowType)
    {
    default:
        frame = Window::Hann(frame);
        break;
    }
    return frame;
}

void PhaseVocoder::Apply(float* buf)
{
    // Find target frequency
    float freqScale = GetFreqScale(buf);
    fprintf(stdout, "%f\n", freqScale); fflush(stdout);
    // Using the frequency scale, we scale the hop size and find the expected time difference between two frames after compressing/stretching the signal
    float dt = ((float)this->frameSize / (float)this->overlap) / (float)this->sampleRate;

    // Slide a window across the input buffer, splitting it into overlapping frames, multiplied by a windowing function if available
    std::vector<std::vector<float>> frames;
    for (int i = 0; i < (int)((float)(this->bufSize - this->frameSize) / (float)(this->frameSize / this->overlap)) + 1; i++)
    {
        int ix = i * (this->frameSize / this->overlap);
        std::vector<float> frame(buf + ix, buf + ix + this->frameSize);
        frame = Window::Hann(frame);
        frames.push_back(frame);
    }
    fprintf(stdout, "windowing %d %d\n", frames.size(), frames[0].size()); fflush(stdout);

    // Apply FFT for every windowed frame and store it
    std::vector<std::vector<std::vector<float>>> stfts;
    float* fpointer;
    for (auto frame : frames)
    {
        //fprintf(stdout, ".\n"); fflush(stdout);
        // Copy the frame to the input buffer
        fpointer = &frame[0];
        memcpy(this->in, fpointer, sizeof(float) * this->frameSize);
        // Apply FFT to windowed sample
        fftwf_execute(this->pf);
        std::vector<std::vector<float>> complex = {};
        for (int i = 0; i < (this->frameSize / 2 + 1); i++)
        {
            std::vector<float> c = {freqs[i][0], freqs[i][1]};
            complex.push_back(c);
        }
        stfts.push_back(complex);
    }
    fprintf(stdout, "stft\n"); fflush(stdout);

    // Adjust the phase of each bin according to the target frequency
    std::vector<float> previousPhase;
    std::vector<float> previousAdjPhase;
    for (int i = 0; i < stfts[0].size(); i++) // Iterate through bins
    {
        previousPhase.push_back(0);
        previousAdjPhase.push_back(0);
        for (int j = 0; j < stfts.size(); j++) // Iterate through windows
        {
            float magnitude = sqrt(stfts[j][i][0] * stfts[j][i][0] + stfts[j][i][1] * stfts[j][i][1]);
            float phase = std::atan2(stfts[j][i][1], stfts[j][i][0]);

            // Frequency Deviation
            //float df = ((phase - previousPhase[i]) / dt) - ((float)i * (float)this->sampleRate / (float)this->frameSize);
            float omega = 2.f * M_PI * ((float)this->frameSize / (float)this->overlap) * ((float)i / (float)this->frameSize);
            float df = fmod(phase - previousPhase[i] - omega + M_PI, 2.f * M_PI) - M_PI;
            float dPhase = omega + df;
            //fprintf(stdout, "%f,", df); fflush(stdout);
            previousPhase[i] = phase;
            //df = fmod(df + M_PI, 2.f * M_PI) - M_PI;
            //float trueFreq = ((float)i * (float)this->sampleRate / (float)this->frameSize) + df;
            //fprintf(stdout, "%f %f,", ((float)i * (float)this->sampleRate / (float)this->frameSize), trueFreq); fflush(stdout);

            // Phase Deviation
            //float adjPhase = previousAdjPhase[i] + (dt * freqScale) * trueFreq;
            float adjPhase = fmod(previousAdjPhase[i] + (dPhase / freqScale) + M_PI, 2.f * M_PI) - M_PI;
            //fprintf(stdout, "%f %f,", phase, adjPhase); fflush(stdout);
            previousAdjPhase[i] = adjPhase;

            // Convert new phase to a complex number a + bi
            stfts[j][i][0] = magnitude * cos(adjPhase);
            stfts[j][i][1] = magnitude * sin(adjPhase);
        }
    }
    fprintf(stdout, "phase adj\n"); fflush(stdout);

    int newSize = ceil((float)bufSize / freqScale);
    //int newSize = bufSize;
    //fprintf(stdout, "%d\n", newSize); fflush(stdout);
    float* output = new float[newSize];
    memset(output, 0, sizeof(float) * newSize);
    float currentIx = 0;
    for (auto &fft : stfts)
    {
        for (int i = 0; i < (this->frameSize / 2 + 1); i++)
        {
            this->freqs[i][0] = fft[i][0];
            this->freqs[i][1] = fft[i][1];
        }
        //this->freqs = &fft[0];
        fftwf_execute(this->pb);
        Window::Hann(out, out, this->frameSize);
        for (int i = 0; i < this->frameSize - 1; i++)
        {
            //fprintf(stdout, "%f,", out[i] / this->frameSize); fflush(stdout);

            output[(int)currentIx + i] += interp(out[i], out[i + 1], ceil(currentIx) - currentIx) / (float)this->frameSize;
        }
        currentIx += (1/freqScale) * (float)this->frameSize / (float)this->overlap;
        /*for (int i = 0; i < this->frameSize; i++)
        {
            //fprintf(stdout, "%f,", out[i]); fflush(stdout);
            output[(int)currentIx + i] += out[i] / (float)this->frameSize;
        }
        currentIx += (float)(this->frameSize / this->overlap);*/
    }
    //if (output[newSize - 1] == 0) newSize--;
    fprintf(stdout, "istft %d\n", newSize); fflush(stdout);

    for (int i = 0; i < this->bufSize; i++)
    {
        //fprintf(stdout, "%f %f %f,", (float)newSize, (float)i, (float(this->bufSize)));
        float pos = (float)newSize * ((float)i / (float)this->bufSize);
        //fprintf(stdout, "%d %f,", i, pos);
        if (ceil(pos) >= newSize) break;
        buf[i] = interp(output[(int)floor(pos)], output[(int)ceil(pos)], pos - floor(pos));
    }
    delete[] output;
    fprintf(stdout, "interp\n"); fflush(stdout);
}

float PhaseVocoder::interp(float a, float b, float d)
{
    return a + (b - a) * d;
}
