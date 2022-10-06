#include "stft.h"

STFT::STFT(int bufSize, int frameFactor, int overlapFactor)
{
    fprintf(stdout, "setting up stft\n"); fflush(stdout);
    this->bufSize = bufSize;
    this->frameFactor = frameFactor;
    this->overlapFactor = overlapFactor;

    updateParams();
}

void STFT::updateParams()
{
    frameSize = bufSize / frameFactor;
    fftSize = frameSize / 2 + 1;
    hopSize = frameSize / overlapFactor;
    nFrames = (bufSize - frameSize) / hopSize + 1;

    in = new float[frameSize];
    fft = new fftwf_complex[fftSize];
    out = new float[frameSize];

    forward = fftwf_plan_dft_r2c_1d(frameSize, in, fft, FFTW_ESTIMATE);
    backward = fftwf_plan_dft_c2r_1d(frameSize, fft, out, FFTW_ESTIMATE);
}

void STFT::setBufSize(int bufSize)
{
    this->bufSize = bufSize;
    updateParams();
}

void STFT::setFrameFactor(int frameFactor)
{
    this->frameFactor = frameFactor;
    updateParams();
}

void STFT::setOverlapFactor(int overlapFactor)
{
    this->overlapFactor = overlapFactor;
    updateParams();
}

fftwf_complex** STFT::stft(float *input)
{
    // Instantiate the output array to the correct size, i.e. number of frames
    fftwf_complex** output = new fftwf_complex*[nFrames];

    for (int i = 0; i < nFrames; i++)
    {
        // Copy part of the input buffer to the fft input
        memcpy(in, input + hopSize * i, sizeof(float) * frameSize);
        Window::Hann(in, in, frameSize); // apply a windowing function
        fftwf_execute(forward);
        output[i] = new fftwf_complex[fftSize]; // create an array to store the fft data
        memcpy(output[i], fft, sizeof(fftwf_complex) * fftSize); // copy the fft data to the buffer
    }
    return output;
}

std::vector<float> STFT::istft(fftwf_complex **input, float timeScale)
{
    int outputSize = (int)std::ceil((float)bufSize * timeScale);
    // nframes = (size - frameSize) / hopSize
    float newHopSize = (float)(outputSize - frameSize) / (float)(nFrames - 1);
    //fprintf(stdout, "%d %f %f %d\n", outputSize, newHopSize, (float)nFrames, outputSize - frameSize); fflush(stdout);
    std::vector<float> output{};
    for (int i = 0; i < outputSize; i++) output.push_back(0.f);

    for (int i = 0; i < nFrames; i++)
    {
        // Copy the current frames' fft to the fft buffer
        memcpy(fft, input[i], sizeof(fftwf_complex) * fftSize);
        fftwf_execute(backward);
        //fprintf(stdout, "reverse fft'd\n"); fflush(stdout);

        // Normalize the output, since the fftw libraries don't normalize in the execution
        for (int j = 0; j < frameSize; j++) out[j] /= frameSize;
        // Apply a window for smoothing
        //Window::Hann(out, out, frameSize);
        //fprintf(stdout, "windowed in place\n"); fflush(stdout);
        // Add to time-stretched buffer
        float offset = (float)i * newHopSize;
        //fprintf(stdout, "%f\n", offset); fflush(stdout);
        if (ceil(offset) == offset) // if the offset is an integer
        {
            for (int j = 0; j < frameSize; j++) output[(int)offset + j] += out[j];
        }
        else // for all other frames, we need to interpolate between points to get an accurate value
        {
            for (int j = 0; j < frameSize - 1; j++)
            {
                if ((int)ceil(offset) + j < outputSize)
                {
                    output[(int)ceil(offset) + j] += std::lerp(out[j], out[j+1], ceil(offset) - offset);
                }
                else
                {
                    break;
                }
            }
            //fprintf(stdout, "added frame 1\n"); fflush(stdout);
        }

        delete[] input[i]; // free memory taken up by the fft data
        //fprintf(stdout, "deleted original frame\n"); fflush(stdout);
    }

    delete[] input; // free memory taken up by the array of fft data -> if istft is not called, user must free this data manually
    //fprintf(stdout, "deleted array of frames\n"); fflush(stdout);
    return output;
}
