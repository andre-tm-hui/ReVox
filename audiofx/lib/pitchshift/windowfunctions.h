#ifndef WINDOWFUNCTIONS_H
#define WINDOWFUNCTIONS_H

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <vector>
#include <stdio.h>
#include <math.h>

namespace Window
{
    static void Hann(float* in, float* out, int n)
    {
        for (int i = 0; i < n; i++)
        {
            out[i] = in[i] * 0.5f * (1.f - cos(2.f * M_PI * (float)i / ((float)n - 1.f)));
        }
        //fprintf(stdout, "\nhann\n"); fflush(stdout);
    }

    static std::vector<float> Hann(std::vector<float> in)
    {
        int L = in.size();
        float N = L - 1.f;
        for (int n = 0; n <= N; n++)
        {
            in[n] *= 0.5f * (1.f - cos(2.f * M_PI * (float)n / N));
        }
        return in;
    }
}

static int argmax(float *arr, int size)
{
    return std::distance(arr, std::max_element(arr, arr + size));
}

#endif // WINDOWFUNCTIONS_H
