#include "frequency.h"

Frequency::Frequency(float bufSize, int resampleFactor) : bufSize(bufSize), resampleFactor(resampleFactor)
{
    this->bufSize = bufSize;
    inputSize = 2 * resampleFactor * bufSize; // The size of the autocorrelation is N + N - 1, so we double the buffer size to get the input size
    fftSize = inputSize / 2 + 1;
    in = new float[inputSize];
    // We set the input array to 0, effectively padding the input array with 0s
    memset(in, 0, sizeof(float) * inputSize);
    out = new float[inputSize];
    fft = new fftwf_complex[fftSize];

    forward = fftwf_plan_dft_r2c_1d(inputSize, in, fft, FFTW_ESTIMATE);
    backward = fftwf_plan_dft_c2r_1d(inputSize, fft, out, FFTW_ESTIMATE);
}

float Frequency::GetPeriod(float *buf)
{
    // Resample input buffer
    int err;
    SRC_STATE *src = src_new(SRC_SINC_FASTEST, 1, &err);
    SRC_DATA *dat = new SRC_DATA();
    dat->data_in = buf;
    dat->data_out = in;
    dat->input_frames = bufSize;
    dat->output_frames = bufSize * resampleFactor;
    dat->src_ratio = resampleFactor;
    memset(in, 0, sizeof(float) * inputSize);
    src_process(src, dat);

    // FFT the input buffer
    //memcpy(in, buf, sizeof(float) * bufSize); // copy the input buffer to the input array, writing over the first N elements of the input array, leaving the rest of the 0s unaffected
    fftwf_execute(forward);
    //fprintf(stdout, "forward\n"); fflush(stdout);

    // Multiply all components by their complex conjugates
    for (int i = 1; i < fftSize; i++)
    {
        fft[i][0] = fft[i][0] * fft[i][0] + fft[i][1] * fft[i][1];
        fft[i][1] = 0.f;
    }
    //fprintf(stdout, "conjugate\n"); fflush(stdout);

    // iFFT to get the autocorrelation
    fftwf_execute(backward);
    //fprintf(stdout, "backwards\n"); fflush(stdout);

    // Find the next highest peak - the highest peak is always at i=0, so the period equals the next highest peak
    int maxIx = inputSize;
    float maxAmp = -INFINITY;
    for (int i = 1; i < inputSize / 2; i++) // halve the search size, since the autocorrelation is circular
    {
        if (out[i] > out[i-1] && out[i] > out[i+1]) // this is a peak
        {
            if (out[i] > maxAmp) // check if the peak is higher than the previously found peak
            {
                maxIx = i;
                maxAmp = out[i];
            }
        }
    }
    //fprintf(stdout, "period found\n"); fflush(stdout);
    return (float)maxIx / (float)resampleFactor; // fundamental frequency = sample rate / period
}
